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
    public:
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
        JModel(JInput const& input_): input(input_){
            init();
            init_heat_sources();
        };
        ~JModel();
        void init();
        void virtual init_heat_sources();
        void switch_models();
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

}


int main(int argc, char const ** argv){
    JInput* input = new JInput(argc, argv);
    input->debug();
}