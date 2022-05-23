#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <stdexcept>
#include <cmath>
#include <chrono>
#include <immintrin.h>
#include <cstdlib>
#include <cstring>
#include <vector>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

/**
 * JSomething stands for
 * jacobi-float-something
 */

class JInput{
    private:
        static const int param_amount = 5;
        static const int exit_status = -1;
        int get_param(char const ** argv, int ind);
    public:
        int Nx;
        int Ny;
        int T;
        int K; // height of iterations "ladder"
        int W = 2; // workers
        bool isVerbose = false;
        JInput(int argc, char const ** argv);
        void debug();
};

JInput::JInput(int argc, char const ** argv){
    if(argc - 1 < 4){
        std::cerr << "Usage: lab1.a [Nx] [Ny] [iTerations] [K-ladder] [Workers] [Verbosity]\n"
            << "\t V>0 - + .txt & deltas in cerr, else silent" << std::endl;
        std::exit(exit_status);
    }
    this->Nx = get_param(argv, 1);
    this->Ny = get_param(argv, 2);
    this->T = get_param(argv, 3);
    this->K = get_param(argv, 4);
    this->W = get_param(argv, 5);
    if(argc == param_amount + 2 && get_param(argv, 6) > 0){
        std::cerr << "Verbosity enabled\n";
        isVerbose = true;
    }
}

int JInput::get_param(char const ** argv, int ind){
    std::string arg = argv[ind];
    try{
        std::size_t pos;
        int x = std::stoi(arg, &pos);
        return x;
    }catch(std::invalid_argument){
        std::cerr << "Invalid number on index " << ind << std::endl;
        std::exit(exit_status);
    }catch(std::out_of_range){
        std::cerr << "OOB on index " << ind << std::endl;
        std::exit(exit_status);
    } 
}

void JInput::debug(){
    std::cout << "JInput: Nx=" << this->Nx << ", Ny=" << this->Ny << ", T=" << this->T << std::endl;
}

////////////////////////////////////

class JModel{
    private:
        float * current_model;
        float * next_model;
        float * heat_sources;
        const float Xa = 0.0f;
        const float Xb = 4.0f;
        const float Ya = 0.0f;
        const float Yb = 4.0f;
        float hx, hy;
        float k1, k2, k3, k4, k5, k6;
        __m256 mm_k2, mm_k3, mm_k4;
        __m256 mask;
    
        void init();
        void virtual init_heat_sources();
        void virtual init_start_values(){};
        void switch_models();
        inline float x(int j){
            return Xa + j*hx;
        }
        inline float y(int i){
            return Ya + i*hy;
        }
        inline int ind(int i, int j){
            return i*input.Nx + j;
        }
        inline float phi_n(int i, int j){
            return *(current_model + ind(i,j));
        }
        inline float p(int i, int j){
            return *(heat_sources + ind(i,j));
        }
        float predict_k_iterations();
        float predict_string(
            int index, 
            float delta, bool isOddIteration);
    public:
        JInput const& input;
        JModel(JInput const& input_): input(input_){
            init();
            init_heat_sources();
            init_start_values();
        };
        ~JModel();
        void predict();
        void dumph();
        void dump();
        void debug();
};

void JModel::init(){
    size_t array_size = input.Nx*input.Ny*sizeof(float);
    current_model = (float*)std::aligned_alloc(32, array_size);
    memset((void*)current_model, 0, array_size);
    next_model = (float*)std::aligned_alloc(32, array_size);
    memset((void*)next_model, 0, array_size);
    heat_sources = (float*)std::aligned_alloc(32, array_size);
    memset((void*)heat_sources, 0, array_size);
    this->hx = (Xb-Xa)/(input.Nx - 1);
    this->hy = (Yb-Ya)/(input.Ny - 1);
    this->k1 = (1.0f/(5.0f/(hx*hx) + 5.0f/(hy*hy)));
    this->k2 = k1*(0.5f)*(5.0f/(hx*hx) - 1.0f/(hy*hy));
    this->k3 = k1*(0.5f)*(5.0f/(hy*hy) - 1.0f/(hx*hx));
    this->k4 = k1*(0.25f)*(1.0f/(hx*hx) + 1.0f/(hy*hy));
    this->k5 = 2.0f*k1;
    this->k6 = 0.25f*k1;

    mm_k2 = _mm256_broadcast_ss(&(JModel::k2));
    mm_k3 = _mm256_broadcast_ss(&(JModel::k3));
    mm_k4 = _mm256_broadcast_ss(&(JModel::k4));
    mask = _mm256_castsi256_ps(_mm256_set1_epi64x(0x7FFFFFFF7FFFFFFF));
}

JModel::~JModel(){
    free(current_model);
    free(next_model);
    free(heat_sources);
}

void JModel::switch_models(){
    float * model_ptr = this->current_model;
    this->current_model = this->next_model;
    this->next_model = model_ptr;
}

void JModel::init_heat_sources(){
    const float Xs1 = Xa + (Xb - Xa)/3;
    const float Xs2 = Xa + (Xb - Xa)*2/3;
    const float Ys1 = Ya + (Yb - Ya)/3;
    const float Ys2 = Ya + (Yb - Ya)*2/3;
    const float R = 0.1f*std::min(std::abs(Xb-Xa), std::abs(Yb-Ya));
    const float source = 0.1f;
    const float sink = -0.1f;

    if(input.isVerbose){std::cerr << Xs1 << " " << Xs2 << " " << Ys1 << " " << Ys2 << " " << R <<std::endl;}
    
    for(int i = 0; i < input.Nx; i++){
        for(int j = 0; j < input.Ny; j++){
            int index = JModel::ind(i,j);
            if( ((x(j) - Xs1)*(x(j) - Xs1) + (y(i) - Ys1)*(y(i) - Ys1)) < R*R){
                heat_sources[index] = source;
                continue;
            }
            if( ((x(j) - Xs2)*(x(j) - Xs2) + (y(i) - Ys2)*(y(i) - Ys2)) < R*R){
                heat_sources[index] = sink;
                continue;
            }
            heat_sources[index] = 0.0f;
        }
    }

    for(int i = 1; i < input.Nx - 1; i++){
        for(int j = 1; j < input.Ny - 1; j++){
            int index = JModel::ind(i,j);
            next_model[index] = JModel::k5*p(i,j) +
                JModel::k6*
                    (p(i - 1, j) + p(i + 1, j) + p(i, j - 1) + p(i, j + 1));
        }
    }

    float* tmp = heat_sources;
    this->heat_sources = next_model;
    this->next_model = tmp;
}

float JModel::predict_string(int index, float delta, bool isOddIteration){
    // supposing that 8 | Nx, Ny; Nx,Ny > 32
    // seven before
    float *phi_ind, *next_ind; 
    float *p_ind = heat_sources + index;

    if(isOddIteration){
        phi_ind = current_model + index;
        next_ind = next_model + index;
    }else{
        phi_ind = next_model + index;
        next_ind = current_model + index;
    }

    for(int j = 1; j < 8; j++){
        *next_ind = 
            JModel::k2*(*(phi_ind - 1) + *(phi_ind + 1)) + 
            JModel::k3*(*(phi_ind - input.Nx) + *(phi_ind + input.Nx)) + 
            JModel::k4*
                (*(phi_ind - input.Nx - 1) + *(phi_ind - input.Nx + 1) + 
                    *(phi_ind + input.Nx - 1) + *(phi_ind + input.Nx + 1)) + 
            *(p_ind);
        float cur_delta = std::abs(next_model[index] - current_model[index]);
        if(cur_delta > delta){
            delta = cur_delta;
        }
        index++;
        next_ind++;
        phi_ind++;
        p_ind++;
    }
    __m256 mm_delta = _mm256_castsi256_ps(_mm256_set1_epi64x(0));
    // cycle (no reusing previous vectors)
    for(int j = 1; j < input.Nx/8 - 1; j++){
        // up/center/down, left/center/right
        __m256 mm_ul = _mm256_loadu_ps(phi_ind - input.Nx - 1);
        __m256 mm_uc = _mm256_load_ps(phi_ind - input.Nx);
        __m256 mm_ur = _mm256_loadu_ps(phi_ind - input.Nx + 1);

        __m256 mm_cl = _mm256_loadu_ps(phi_ind - 1);
        __m256 mm_cc = _mm256_load_ps(phi_ind);
        __m256 mm_cr = _mm256_loadu_ps(phi_ind + 1);

        __m256 mm_dl = _mm256_loadu_ps(phi_ind + input.Nx - 1);
        __m256 mm_dc = _mm256_load_ps(phi_ind + input.Nx);
        __m256 mm_dr = _mm256_loadu_ps(phi_ind + input.Nx + 1);

        __m256 mm_p = _mm256_load_ps(p_ind);

        __m256 mm_res = 
            _mm256_fmadd_ps(
                mm_k2, (mm_cl + mm_cr),
                _mm256_fmadd_ps(
                    mm_k3, (mm_uc + mm_dc),
                    mm_k4*(mm_ul + mm_ur + mm_dl + mm_dr + mm_p)
                )
            );
        
        _mm256_store_ps(next_ind, mm_res);

        // finding max, mask declared at start
        mm_delta = _mm256_max_ps(_mm256_and_ps(_mm256_sub_ps(mm_res, mm_cc),mask), mm_delta);

        index += 8;
        next_ind += 8;
        phi_ind += 8;
        p_ind += 8;

    }
    // cycle after
    for(int j = input.Nx - 8; j < input.Nx - 1; j++){
        *next_ind = 
            JModel::k2*(*(phi_ind - 1) + *(phi_ind + 1)) + 
            JModel::k3*(*(phi_ind - input.Nx) + *(phi_ind + input.Nx)) + 
            JModel::k4*
                (*(phi_ind - input.Nx - 1) + *(phi_ind - input.Nx + 1) + 
                    *(phi_ind + input.Nx - 1) + *(phi_ind + input.Nx + 1)) + 
            *(p_ind);
        float cur_delta = std::abs(next_model[index] - current_model[index]);
        if(cur_delta > delta){
            delta = cur_delta;
        }
        index++;
        next_ind++;
        phi_ind++;
        p_ind++;

    }

    //checking in vector delta - slow, but once:
    alignas(32) float arr[8];
    _mm256_store_ps(arr, mm_delta);
    for(int i = 0; i < 8; i++){
        delta = (arr[i] > delta ? arr[i] : delta);
    }

    return delta;
}

float JModel::predict_k_iterations(){
    // supposing that 8 | Nx, 8 | Ny - correct for 10000
    float delta = 0.0f;
    int index;    

    /**
     * starting slope
     */

    for(int i = 1; i <= input.K; i++){
        for(int j = 1; j <= input.K + 1 - i; j++){
            delta = predict_string(JModel::ind(j,1), delta, i % 2);
        }
    }
    // main ladder

    for(int i = 2; i < input.Ny - 1; i++){
        for(int j = 1; j <= input.K; j++){
            int string = i + input.K - j;
            if(string < input.Ny - 1){
                delta = predict_string(JModel::ind(string, 1), delta, j % 2);
            }
        }
    }

    return delta;
}

void JModel::predict(){
    float delta, prev_delta = 0.0f;
    /**
     * now we are predicting T/K iterations
     * with ladder of height of K
     * oh and one last thing
     * K is expected to be even and K | T
     */ 
    for(int i = 0; i < input.T/input.K; i++){
        delta = predict_k_iterations();
        //switch_models();

        //if(input.isVerbose){std::cerr << delta << std::endl;}

        if(delta > prev_delta && std::abs(delta - prev_delta) > 10e-9 && i > 1){
            std::cerr << "Delta error on iteration " << i+1 << " with " << delta << std::endl;
            JModel::dump();
            std::exit(-1);
        }

        if(input.isVerbose){
            std::cerr << "Delta on " << input.K << "-iteration pack " << i+1 << " equals " << delta << std::endl;
        }
        prev_delta = delta;
    }
}

void JModel::dumph(){
    std::ofstream fout("out.txt");
    for(int i = 0; i < input.Ny; i++){
        for(int j = 0; j < input.Nx; j++){
            fout << phi_n(i,j) << " ";
        }
        fout << std::endl;
    }
    fout.close();
}

void JModel::dump(){
    if(std::FILE* f1 = std::fopen("out.bin", "wb")) {
        std::fwrite(static_cast<void*>(current_model), sizeof(float), input.Nx*input.Ny, f1);
        std::fclose(f1);
    }
    // big oof
    // if(input.isVerbose){
    //     JModel::dumph();
    // }
}



class JThreads{
public:
    std::vector<std::thread> thr;
    std::mutex m;
    std::condition_variable cv;
    //TODO: bools are shaky, mb ints are better solution
    std::vector<std::unique_ptr<std::atomic_bool>> is_green_ready; //red->blue needs next green
    std::vector<std::unique_ptr<std::atomic_bool>> is_blue_ready;  //blue->green needs prev blue done
    bool ready = false;
    JModel& model;

    JThreads(JModel& model_): model(model_){
        thr.push_back(std::thread(&JThreads::first_worker, this));
        for(int i = 1; i < model.input.W - 1; ++i){
            thr.push_back(std::thread(&JThreads::worker, this, i));
        }
        thr.push_back(std::thread(&JThreads::last_worker, this));
        for(int i = 0; i < model.input.W; ++i){
            is_green_ready.push_back(std::make_unique<std::atomic_bool>(false));
            is_blue_ready.push_back(std::make_unique<std::atomic_bool>(true));
        }
    }
    void join(){
        for(auto& i : thr){
            i.join();
        }
    }

    void predict();

    void first_worker();
    void worker(int id);
    void last_worker();

    inline void first_predict_green(int id);
    inline void common_predict_green(int id);
    inline void predict_red(int id);
    inline void common_predict_blue(int id);
    inline void last_predict_blue(int id);

    inline void acquire_blue_perm(int id);
    inline void acquire_green_perm(int id);
    inline void give_blue_perm(int id);
    inline void give_green_perm(int id);

    void wait_for_start();

    inline int get_shift_for_id(int id);
    inline int get_length_for_id(int id);
};

void JThreads::first_worker(){
    int id = 0;
    wait_for_start();
    std::cerr << "[" + std::to_string(id) + "]: Start\n";
    for(int i = 0; i < model.input.T/model.input.K; ++i){
        first_predict_green(id);
        predict_red(id);
            acquire_blue_perm(id);
        common_predict_blue(id);
            give_green_perm(id);
    }
    std::cerr << "[" + std::to_string(id) + "]: Finish\n";
}

void JThreads::worker(int id){
    wait_for_start();
    std::cerr << "[" + std::to_string(id) + "]: Start\n";
    for(int i = 0; i < model.input.T/model.input.K; ++i){
            acquire_green_perm(id);
        common_predict_green(id);
            give_blue_perm(id);
        predict_red(id);
            acquire_blue_perm(id);
        common_predict_blue(id);
            give_green_perm(id);
    }
    std::cerr << "[" + std::to_string(id) + "]: Finish\n";
}

void JThreads::last_worker(){
    int id = model.input.W - 1;
    std::cerr << "[" + std::to_string(id) + "]: Start\n";
    for(int i = 0; i < model.input.T/model.input.K; ++i){
            acquire_green_perm(id);
        common_predict_green(id);
            give_blue_perm(id);
        predict_red(id);
        last_predict_blue(id);
    }
    std::cerr << "[" + std::to_string(id) + "]: Finish\n";
}

inline void JThreads::acquire_blue_perm(int id){
    // wait for next's green to be ready
    bool can_proceed = false;
    while(!can_proceed){
        can_proceed = is_green_ready.at(id + 1)->load();
    }
    is_green_ready.at(id + 1)->store(false);
}
inline void JThreads::acquire_green_perm(int id){
    // wait for prev's blue to be ready
    // so our green is no longer needed
    bool can_proceed = false;
    while(!can_proceed){
        can_proceed = is_blue_ready.at(id - 1)->load();
    }
    is_blue_ready.at(id - 1)->store(false);
}
inline void JThreads::give_blue_perm(int id){
    // tell prev that our green are ready
    is_green_ready.at(id)->store(true);
}
inline void JThreads::give_green_perm(int id){
    // tell next that our blue are reary
    // so his green can be updated
    is_blue_ready.at(id)->store(true);
}

inline void JThreads::first_predict_green(int id){
    std::cerr << "[" + std::to_string(id) + "]: green\n";
}
inline void JThreads::common_predict_green(int id){
    std::cerr << "[" + std::to_string(id) + "]: green\n";
}
inline void JThreads::predict_red(int id){
    std::cerr << "[" + std::to_string(id) + "]: red\n";
}
inline void JThreads::common_predict_blue(int id){
    std::cerr << "[" + std::to_string(id) + "]: blue\n";
}
inline void JThreads::last_predict_blue(int id){
    std::cerr << "[" + std::to_string(id) + "]: blue\n";
}

void JThreads::wait_for_start(){
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, [&]{return ready;});
}

void JThreads::predict(){
    {
        std::lock_guard<std::mutex> lk(m);
        ready = true;
    }
    cv.notify_all();
}

///////////////

int main(int argc, char const ** argv){
    //std::cout << sizeof(float) << " " << sizeof(__m256) << std::endl;
    //sizeof(float) - 4, sizeof(__mm256) - 32 => 8 floats in vec
    std::cerr << std::setprecision(12) ;


    auto input = std::make_unique<JInput>(argc, argv);
    input->debug();
    auto model = std::make_unique<JModel>(*input);
    auto threads = std::make_unique<JThreads>(*model);

    auto start = std::chrono::steady_clock::now();
    threads->predict();
    auto end = std::chrono::steady_clock::now();

    std::chrono::duration<double> diff = end - start;

    threads->join();

    std::cout << "Time to predict: " << diff.count() << " s\n";

    std::cout << "Success! Go check .png" << std::endl;
    model->dump();
}