#include <iostream>
#include <string>
#include <stdexcept>

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
        inline float x(int i){
            return Xa + i*hx;
        }
        inline float y(int j){
            return Ya + j*hy;
        }
        inline float phi_n(int i, int j){
            return *(current_model + i*input.Ny + j);
        }
        inline float p(int i, int j){
            return *(heat_sources + i*input.Ny + j);
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
        void dump();
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

    for(int i = 0; i < input.Nx; i++){
        for(int j = 0; j < input.Ny; j++){
            int ind = i*input.Ny + j;
            if( (x(i) - Xs1)*(x(i) - Xs1) + (y(i) - Ys1)*(y(i) - Ys1) < R*R){
                heat_sources[ind] = source;
                continue;
            }
            if( (x(i) - Xs2)*(x(i) - Xs2) + (y(i) - Ys2)*(y(i) - Ys2) < R*R){
                heat_sources[ind] = sink;
                continue;
            }
            heat_sources[ind] = 0.0f;
        }
    }
}

float JModel::predict_iteration(){
    //TODO:stub; returns max delta
    return 0.0;
}

void JModel::predict(){
    float delta, prev_delta = 0.0f
    for(int i = 0; i < this->input.T; i++){
        delta = predict_iteration();
        switch_models();
        if(delta > prev_delta && i > 0){
            std::cerr << "Delta error on iteration " << i+1 << std::endl;
            std::exit(-1);
        }
    }
}

void JModel::dump(){
    // TODO:stub
    // dumps model in file for visualisation
}

///////////////

int main(int argc, char const ** argv){
    JInput* input = new JInput(argc, argv);
    input->debug();
    JModel* model = new JModel(*input);
    model->predict();
    model->dump();
}