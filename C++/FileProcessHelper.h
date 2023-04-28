#ifndef __FILE_PROCESS_HELPER_H__
#define __FILE_PROCESS_HELPER_H__
#include "CommonHeaders.h"


struct FileProcessHelper{


    std::string SplitFilename(char* argv[]);
    int CheckColorMapValidation(const char* input);
    void FilenameGenerator( const std::string& filename, 
                            std::vector<std::string>& filename_storage, 
                            int num_of_layer);

    int ColorMapGenerator(  int argc, 
                            char* argv[], 
                            size_t start_index, 
                            size_t end_index,
                            std::vector<std::string>& colormap_storage, 
                            size_t num_of_layer);

};




#endif