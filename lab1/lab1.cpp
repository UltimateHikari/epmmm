#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <cmath>
#include <chrono>

/**
 * JSomething stands for
 * jacobi-float-something
 */

class JInput{
    private:
        static const int param_amount = 3;
        static const int exit_status = -1;
        int get_param(char const ** argv, int ind);
    public:
        int Nx;
        int Ny;
        int T;
        JInput(int argc, char const ** argv);
        void debug();
};

JInput::JInput(int argc, char const ** argv){
    if(argc - 1 < 3){
        std::cerr << "Usage: lab1.a [Nx] [Ny] [T]" << std::endl;
        std::exit(exit_status);
    }
    this->Nx = get_param(argv, 1);
    this->Ny = get_param(argv, 2);
    this->T = get_param(argv, 3);
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
        JInput const& input;
        float * current_model;
        float * next_model;
        float * heat_sources;
        const float Xa = 0.0f;
        const float Xb = 4.0f;
        const float Ya = 0.0f;
        const float Yb = 4.0f;
        float hx;
        float hy;
    
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
        float predict_iteration();
    public:
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
    current_model = new float[input.Nx*input.Ny];
    next_model = new float[input.Nx*input.Ny];
    heat_sources = new float[input.Nx*input.Ny];
    this->hx = (Xb-Xa)/(input.Nx - 1);
    this->hy = (Yb-Ya)/(input.Ny - 1);
}

JModel::~JModel(){
    delete current_model;
    delete next_model;
    delete heat_sources;
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
    const float R = 0.1*std::min(std::abs(Xb-Xa), std::abs(Yb-Ya));
    const float source = 0.1f;
    const float sink = -0.1f;
    //std::cerr << Xs1 << " " << Xs2 << " " << Ys1 << " " << Ys2 << " " << R <<std::endl;
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
}

float JModel::predict_iteration(){
    float delta = 0.0f;
    for(int i = 1; i < input.Ny - 1; i++){
        for(int j = 1; j < input.Nx - 1; j++){
            int index = JModel::ind(i,j);
            next_model[index] = 
            (1.f/(5.f/(hx*hx) + 5.f/(hy*hy)))*(
                (.5f)*(5.f/(hx*hx) - 1.f/(hy*hy))*(phi_n(i, j - 1) - phi_n(i, j + 1)) + 
                (.5f)*(5.f/(hy*hy) - 1.f/(hx*hx))*(phi_n(i - 1, j) - phi_n(i + 1, j)) + 
                (.25f)*(1.f/(hx*hx) + 1.f/(hy*hy))*
                    (phi_n(i - 1, j - 1) + phi_n(i - 1, j + 1) + phi_n(i + 1, j - 1) + phi_n(i + 1, j + 1)) + 
                2*p(i,j) +
                (.25f)*
                    (p(i - 1, j) + p(i - 1, j) + p(i, j - 1) + p(i, j + 1))
            );
            float cur_delta = std::fabs(next_model[index] - current_model[index]);
            if(cur_delta > delta){
                delta = cur_delta;
            }
        }
    }
    return delta;
}

void JModel::predict(){
    float delta, prev_delta = 0.0f;
    for(int i = 0; i < this->input.T; i++){
        delta = predict_iteration();
        switch_models();
        //std::cerr << delta << "; ";
        if(delta > prev_delta && i > 0){
            std::cerr << "Delta error on iteration " << i+1 << " with " << delta << std::endl;
            JModel::dump();
            std::exit(-1);
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
}

///////////////

int main(int argc, char const ** argv){
    JInput* input = new JInput(argc, argv);
    input->debug();
    JModel* model = new JModel(*input);

    auto start = std::chrono::steady_clock::now();
        model->predict();
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Time to predict: " << diff.count() << " s\n";

    std::cout << "Success! Go check .png" << std::endl;
    model->dump();
    model->dumph();
}