#ifndef __COMMAND_PROCESS_HELPER_H__
#define __COMMAND_PROCESS_HELPER_H__

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <unistd.h>
/*
Helper class for execute the command.
This is a singleton class.
Initialization: filename, 
                generated file name storage by function FilenameGenerator
                generated user color preference by function ColorMapGenerator
                Number of layers of the NRRD image
*/


struct CommandProcessHelper{
    static CommandProcessHelper& getInstance(const std::string& filename, 
                                const std::vector<std::string>& filename_storage,
                                const std::vector<std::string>& user_color_storage,
                                int num_of_layer);
    void Command_SetMain(int filename_storage_index); //do -layers-set-main
    void Command_AddAnat(int filename_storage_index); //do -layers-add-anat
    void Command_SetColorMap(int color_map_index); //do -props-set-colormap

    void Command_CreateNewWorkSpaceFile(); //do -o and save

    int Command_Execute(); //execute command

    void Command_Serialize(); //serialize command

    const std::string&              _filename;
    const std::vector<std::string>& _filename_storage;
    const std::vector<std::string>& _user_color_storage;
    int _num_of_layer;
    std::stringstream _ss;

    private:
    CommandProcessHelper(   const std::string& filename, 
                            const std::vector<std::string>& filename_storage, 
                            const std::vector<std::string>& user_color_storage,
                            int num_of_layer)
    :_filename(filename), _filename_storage(filename_storage),_user_color_storage(user_color_storage), 
    _num_of_layer(num_of_layer), _ss{}
    {}
    
    CommandProcessHelper(const CommandProcessHelper&) = delete;
    CommandProcessHelper& operator=(const CommandProcessHelper&) = delete;
    
    // bool Command_AnalyseUserColorSelection(const char* user_selection); //
    /*
    *check the <filename>.nrrd is exist or not
    !Note Take care, this check file name I only use once. 
    It means filename.nrrd if exist, user choose not to overwrite it,
    filename_1.nrrd will be generated. However, if next time user still use filename.nrrd as input, and user choose not to overwrite it,
    but this time filename_1.nrrd is exist, so the filename_1.nrrd will be re-write. TL,DR it will only check first user's input.
    To solve this issue, just simply use a while loop. But I decide not to do that.
    */
    bool Command_CheckFileExistance(const std::string& filename);
    void Serialize_ColorEmpty(); //for user has no color preference
    void Serialize_ColorNotEmpty(); //for user has color preference

};

#endif