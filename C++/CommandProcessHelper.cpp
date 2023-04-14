#include "CommandProcessHelper.h"

CommandProcessHelper& CommandProcessHelper::getInstance(const std::string& filename, 
                            const std::vector<std::string>& filename_storage, 
                            const std::vector<std::string>& user_color_storage,
                            int num_of_layer){
    static CommandProcessHelper instance(filename, filename_storage, user_color_storage, num_of_layer);
    return instance;
}
void CommandProcessHelper::Command_SetMain(int filename_storage_index){
    _ss << "itksnap-wt -layers-set-main " << _filename_storage[filename_storage_index];
}
void CommandProcessHelper::Command_AddAnat(int filename_storage_index){
    _ss << " -layers-add-anat " << _filename_storage[filename_storage_index];
}
void CommandProcessHelper::Command_SetColorMap(int color_map_index){
    _ss << " -props-set-colormap " << "\"" << _user_color_storage[color_map_index] << "\"";
}

void CommandProcessHelper::Command_CreateNewWorkSpaceFile(){
    if(Command_CheckFileExistance(_filename) == true){
        int instruction;
        std::cout <<"File " << _filename <<".itksnap is already exists. If you want to overwrite it, please press 0. Else, please press 1" << std::endl;
        std::cin >> instruction;
        if(instruction == 0){
            _ss << " -o " << _filename << ".itksnap";
        }
        else{
            _ss << " -o " << _filename << "_1.itksnap";
        }
    }
    else{
        _ss << " -o " << _filename << ".itksnap";
    }
}

int CommandProcessHelper::Command_Execute(){
    std::string command = _ss.str();
    //std::cout << command << std::endl;
    const char *final = command.c_str();
    int status = system(final);
    if(status == -1){
        std::cerr <<"execute command fail for stage 1" << std::endl;
        return -1;
    }
    else{
        if(!WIFEXITED(status)){
            std::cerr <<"execute command fail for stage 2" << std::endl;
            return -1;
        }
        else{
            if(WEXITSTATUS(status)){
                std::cerr <<"execute command fail for stage 3" << std::endl;
                return -1;
            }
            else{
                std::cout <<"execute command success" << std::endl;
            }
        }
    }
    return 0;

}

void CommandProcessHelper::Command_Serialize(){
    if(_user_color_storage.empty() == true){
        Serialize_ColorEmpty();
    }
    else{
        Serialize_ColorNotEmpty();
    }
}

void CommandProcessHelper::Serialize_ColorEmpty(){
    int cur_index = 0;
    Command_SetMain(cur_index); //set main
    cur_index++;
    for(; cur_index < _num_of_layer; ++cur_index){
        Command_AddAnat(cur_index); //add additional layers
    }
    Command_CreateNewWorkSpaceFile();
}
void CommandProcessHelper::Serialize_ColorNotEmpty(){
    int cur_index = 0;
    Command_SetMain(cur_index); //set main
    Command_SetColorMap(cur_index);
    cur_index++;
    for(; cur_index < _num_of_layer; ++cur_index){
        Command_AddAnat(cur_index); //add additional layers
        Command_SetColorMap(cur_index);
    }
    Command_CreateNewWorkSpaceFile();
}


// bool CommandProcessHelper::Command_AnalyseUserColorSelection(const char* user_selection){
//     std::string userColorInput = user_selection;
//     if(userColorInput.size() < _num_of_layer){
//         std::cout <<"Your color input is less then the number of layers of the image. Rest of layers will be displayed by default gray color.\n";
//     }
//     return false;

// }




/*
*check the <filename>.nrrd is exist or not
!Note Take care, this check file name I only use once. 
It means filename.nrrd if exist, user choose not to overwrite it,
filename_1.nrrd will be generated. However, if next time user still use filename.nrrd as input, and user choose not to overwrite it,
but this time filename_1.nrrd is exist, so the filename_1.nrrd will be re-write. TL,DR it will only check first user's input.
To solve this issue, just simply use a while loop. But I decide not to do that.
*/
bool CommandProcessHelper::Command_CheckFileExistance(const std::string& filename){
    std::string s_file_name = filename + ".itksnap";
    const char* check_file_name = s_file_name.c_str();
    //std::cout <<"in check" << std::endl << check_file_name << std::endl;
    if(access(check_file_name, F_OK) == 0){
        //std::cout <<"file exist" << std::endl;
        return true;
    }
    else{
        //std::cout <<"file not exist" << std::endl;
        return false;
    }
}