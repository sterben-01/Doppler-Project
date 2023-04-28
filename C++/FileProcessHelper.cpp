#include"FileProcessHelper.h"

/*
color map description.
the second variable in the map is the index of the ColorMaps array.
*/
const std::unordered_map<const char*, int> AbbrColorMaps{
    {"Gr", 0},
    {"H", 2},
    {"C", 3},
    {"R", 4},
    {"G", 5},
    {"B", 6},
    {"Grayscale", 0},
    {"Jet", 1},
    {"Hot", 2},
    {"Cool", 3},
    {"Black to red", 4},
    {"Black to green", 5},
    {"Black to blue", 6},
    {"Spring", 7},
    {"Summer", 8},
    {"Autumn", 9},
    {"Winter", 10},
    {"Copper", 11},
    {"HSV", 12},
    {"Blue to white to red", 13},
    {"Red to white to blue", 14},
    {"Speed image (blue to black to white)", 15},
    {"Speed image (semi-transparent overlay)", 16},
    {"Level set image", 17}
};
constexpr std::array<const char*, 18> ColorMaps{     
    "Grayscale",
    "Jet",
    "Hot",
    "Cool",
    "Black to red",
    "Black to green",
    "Black to blue",
    "Spring",
    "Summer",
    "Autumn",
    "Winter",
    "Copper",
    "HSV",
    "Blue to white to red",
    "Red to white to blue",
    "Speed image (blue to black to white)",
    "Speed image (semi-transparent overlay)",
    "Level set image"
};


/*
*Split the filename from user input
&Sample: user input test.nrrd, it will extract the test out of the string
@Argument: argv
@Return: extracted filename
*/
std::string FileProcessHelper::SplitFilename(char* argv[]){
    std::string inputFilename = argv[1];
    size_t pos = inputFilename.find('.');
    if(pos != std::string::npos){
        return inputFilename.substr(0, pos);
    }
    return {};
}

/*
*check the user input is already in the color map presets or not
@Argument: user input
@Return: If find, return the index of the user input in the preset, if not exist, return -1.
*/
int FileProcessHelper::CheckColorMapValidation(const char* input){
    for(auto& color : AbbrColorMaps){
        if (strcmp(input, color.first) == 0) {
            return color.second;
        }
    }
    return -1;
}

/*
*Generate splitted file names from the file name
&Sample: user input test.nrrd, it will have from test_splitted_1.nrrd to test_splitted_<n>.nrrd
@Argument: filename(user's file name), filename_storage(will store generated filenames), num_of_layer (layers' number of the image)
@Return: None
*/
void FileProcessHelper::FilenameGenerator(  const std::string& filename, 
                                            std::vector<std::string>& filename_storage, 
                                            int num_of_layer){
    for(int i = 0; i < num_of_layer; i++){
        filename_storage.emplace_back(filename + "_splitted_" + std::to_string(i+1) + ".nrrd");
    }
}

/*
*Generate and find correct colormap from user input
@Argument: argc, argv, start_index (argv's start pos), end_index(argv's end pos), colormap_storage(will store generated colormap), num_of_layer (layers' number of the image)
@Return: None
*/
int FileProcessHelper::ColorMapGenerator(int argc, 
                                        char* argv[], 
                                        size_t start_index, 
                                        size_t end_index,
                                        std::vector<std::string>& colormap_storage, 
                                        size_t num_of_layer){
    if(start_index >= argc){
        std::cout <<"You didn't enter color preference, it will no configuration for the color map\n";
        return 0;
    }
    if((end_index - start_index) > num_of_layer){
        std::cout <<"You entered too much colors.\n. Program stopping.......\n";
        return -1;
    }
    size_t accumulate = 0;
    for(size_t i = start_index; i < end_index; ++i){
        int index = CheckColorMapValidation(argv[i]);
        if(index == -1){
            std::cout << "You entered invalid color preference which is: "<< argv[i] <<", it will be replaced to default gray color.\n";
            colormap_storage.emplace_back("Grayscale");
        }
        else{
            colormap_storage.emplace_back(ColorMaps[index]);
        }
        ++accumulate;
    }
    if((end_index - start_index) < num_of_layer){
        std::cout <<"Your color input is less then the number of layers of the image. Rest of layers will be displayed by default gray color.\n";
        while(accumulate < num_of_layer){
            colormap_storage.emplace_back("Grayscale");
            ++accumulate;
        }
    }
#ifdef DEBUG_MODE
    if(colormap_storage.size() == num_of_layer){
        std::cout <<"Filename Passed\n";
    }
    else{
        std::cout <<"FILENAME DIDN'T PASS!!!!!\n";
        return -1;
    }
#endif
    assert(colormap_storage.size() == num_of_layer);
    return 0;
}

