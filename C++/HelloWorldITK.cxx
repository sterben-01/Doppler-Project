#include <itkImage.h>
#include <itkMacro.h>
#include <itkImageFileWriter.h>
#include <itkImageFileReader.h>
#include <itkMetaDataObject.h>
#include <itkNrrdImageIO.h>
#include "itkImageAdaptor.h"
#include "itkImageRegionIterator.h"
#include "itkNthElementImageAdaptor.h"
#include "itkVectorIndexSelectionCastImageFilter.h"
#include "CommandProcessHelper.h"
#include <cstdlib>
#include <future>
#include <thread>
#include <mutex>
#include <array>
#include <unordered_map>
#include "HelpInfo.cpp"
/*
color map description.
the second variable in the map is the index of the ColorMaps array.
*/
static std::unordered_map<const char*, int> AbbrColorMaps{
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
constexpr static std::array<const char*, 18> ColorMaps{     
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



template<unsigned int Channels>
struct ChannelTypeTraits{
    using VectorType            = itk::Vector<unsigned char, Channels>;      //Number of the Channel
    using SplitVectorType       = itk::Vector<unsigned char, 1>;             //Vector type after separation
    using DiffusionImageType    = itk::Image<VectorType, Channels>;          //How many views 
    using SplitImageType        = itk::Image<SplitVectorType, Channels>;     //Image type after separation
    using ImageExtractorType    = itk::VectorIndexSelectionCastImageFilter<DiffusionImageType, SplitImageType>;
    using DiffusionImagePointer = typename DiffusionImageType::Pointer;
    using FileReaderType        = itk::ImageFileReader<DiffusionImageType>;
    using WriterType            = itk::ImageFileWriter<SplitImageType>;

    static void func(){
        std::cout << Channels << std::endl;
    }
};

/*
Do Image Extractions
*/
template<unsigned int Channels>
static void ImageExtraction(typename ChannelTypeTraits<Channels>::ImageExtractorType::Pointer image_extractor, 
                            typename ChannelTypeTraits<Channels>::FileReaderType::Pointer input_image, 
                            int index){
    image_extractor->SetInput(input_image->GetOutput());
    image_extractor->SetIndex(index);
    image_extractor->Update();


}
/*
Write splitted files
*/
template<unsigned int Channels>
static void WriteFile(  typename ChannelTypeTraits<Channels>::WriterType::Pointer writer, 
                        typename ChannelTypeTraits<Channels>::ImageExtractorType::Pointer image_extractor, 
                        const std::vector<std::string>& filename_storage, 
                        int index){

    writer->SetInput(image_extractor->GetOutput());
    writer->SetFileName(filename_storage[index]);
    writer->Update();

}

/*
*do the image extraction and write file
@Argument: argc, order(the nth layer of image will be extracted), filename_storage, index(the index of the filename_storage. Correspond to the nth filename)
*/
template<unsigned int N>
void Process(char* argv[], int order, const std::vector<std::string>& filename_storage, int index){
    thread_local typename ChannelTypeTraits<N>::FileReaderType::Pointer reader = ChannelTypeTraits<N>::FileReaderType::New();
    reader->SetFileName(argv[1]);
    reader->Update();
    thread_local itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New(); //I think thread_local is necessary

    // This statement is to be added to the ITK CVS Release
    io->SetFileType(itk::ImageIOBase::ASCII); //I don't know this kind of IO sharing will cause race condition or not. Waiting for checking.

    thread_local typename ChannelTypeTraits<N>::ImageExtractorType::Pointer image_extractor  = ChannelTypeTraits<N>::ImageExtractorType::New();
    thread_local typename ChannelTypeTraits<N>::WriterType::Pointer         nrrd_writer = ChannelTypeTraits<N>::WriterType::New();
    nrrd_writer->SetImageIO(io);

    ImageExtraction<N>(image_extractor, reader, order);
    WriteFile<N>(nrrd_writer, image_extractor, filename_storage, index);

}

/*
* use several threads to process image extraction
!NOTE: Currently only support up to 6 layers, so maximum 6 threads will be created
index is start from zero. This num is use to specify the index of the nrrd file which we want to keep.
the another usage of it is to store std::thread object to an array.
index also is the formatted file name index in the filename_storage
@Argument: argc, argv, filename_storage

*/
template<unsigned int N>
void Selection(int argc, char* argv[], const std::vector<std::string>& filename_storage){
    std::thread my_thcontainer[N];
    size_t index = 0;
    for(; index < N; index++){
        my_thcontainer[index] = std::thread(Process<N>, argv, index, filename_storage, index);

    }
    for(auto& item:my_thcontainer){
        item.join();
    }
}

/*
*Split the filename from user input
&Sample: user input test.nrrd, it will extract the test out of the string
@Argument: argv
@Return: extracted filename
*/
std::string SplitFilename(char* argv[]){
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
int CheckColorMapValidation(const char* input){
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
void FilenameGenerator(const std::string& filename, std::vector<std::string>& filename_storage, int num_of_layer){
    for(int i = 0; i < num_of_layer; i++){
        filename_storage.emplace_back(filename + "_splitted_" + std::to_string(i+1) + ".nrrd");
    }
}
/*
*Generate and find correct colormap from user input
@Argument: argc, argv, start_index (argv's start pos), end_index(argv's end pos), colormap_storage(will store generated colormap), num_of_layer (layers' number of the image)
@Return: None
*/

int ColorMapGenerator(  int argc, 
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
    if(colormap_storage.size() == num_of_layer){
        std::cout <<"Filename Passed\n";
    }
    else{
        std::cout <<"FILENAME DIDN'T PASS!!!!!\n";
        return -1;
    }
    return 0;
}


int main(int argc, char* argv[])
{  
    if(argc <= 1){
        std::cout <<"You can use <-h> to view help documentation" << std::endl;
        std::cout <<"Basic format: <Original NRRD file name>, <ColorOption>..." << std::endl;
        return -1;
    }
    if(argc == 2 && strcmp(argv[1], "-h") == 0){
        HelpInfo::printHelperInfo();
        return -1;
    }
    std::string filename = SplitFilename(argv);
    //std::cout << filename << std::endl;
    if(filename.empty()){
        std::cout <<"Please enter the correct NRRD file. If you don't understand how to use, enter <-h>" << std::endl;
        return -1;
    }

    itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
    io->SetFileName(argv[1]);
    io->ReadImageInformation();
    const unsigned int dimensionality = io->GetNumberOfDimensions();
    const unsigned int number_of_components = io->GetNumberOfComponents();
    std::cout << "Dimension: "<<dimensionality << std::endl;
    std::cout << "Number of component: "<<number_of_components << std::endl;
    if(number_of_components > 6){
        std::cout <<"Not support this nums of layer, it should be 2 ~ 6" << std::endl;
        return -1;
    }


    std::vector<std::string>filename_storage;
    filename_storage.reserve(number_of_components);
    FilenameGenerator(filename, filename_storage, number_of_components);

    std::vector<std::string>colormap_storage;
    colormap_storage.reserve(number_of_components);
    if(ColorMapGenerator(argc, argv, 2, argc, colormap_storage, number_of_components) == -1){
        return -1;
    } //! note the 4th argument currently is equal to the number of argument. But in the future, it could be changed.


    if(number_of_components == 1){
        //Nothing need to do for Single layer image
    }
    else if(number_of_components == 2){
        Selection<2>(argc, argv, filename_storage);
    }
    else if(number_of_components == 3){
        Selection<3>(argc, argv, filename_storage);
    }
    else if(number_of_components == 4){
        Selection<4>(argc, argv, filename_storage);
    }
    else if(number_of_components == 5){
        Selection<5>(argc, argv, filename_storage);
    }
    else if(number_of_components == 6){
        Selection<6>(argc, argv, filename_storage);
    }

    CommandProcessHelper& cmdhelper = CommandProcessHelper::getInstance(filename,
                                                                        filename_storage,
                                                                        colormap_storage,
                                                                        number_of_components);

    cmdhelper.Command_Serialize();
    cmdhelper.Command_Execute();
    return 0;
}
