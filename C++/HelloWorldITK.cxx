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
#include <thread>
#include "HelpInfo.cpp"
#include "CommandProcessHelper.h"
#include "FileProcessHelper.h"


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

int GetNumberOfInputFile(char* argv[], unsigned int& dimensionality, unsigned int& number_of_components){
    itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
    io->SetFileName(argv[1]);
    io->ReadImageInformation();
    dimensionality = io->GetNumberOfDimensions();
    number_of_components = io->GetNumberOfComponents();
    std::cout << "Dimension: "<<dimensionality << std::endl;
    std::cout << "Number of component: "<<number_of_components << std::endl;
    if(number_of_components > 6){
        std::cout <<"Not support this nums of layer, it should be 2 ~ 6" << std::endl;
        return -1;
    }
    return 0;
}

int main(int argc, char* argv[]){  
    if(argc <= 1){
        std::cout <<"You can use <-h> to view help documentation" << std::endl;
        std::cout <<"Basic format: <Original NRRD file name>, <ColorOption>..." << std::endl;
        return -1;
    }
    if(argc == 2 && strcmp(argv[1], "-h") == 0){
        HelpInfo::printHelperInfo();
        return -1;
    }

    FileProcessHelper filehelper; 
    std::string filename = filehelper.SplitFilename(argv);
    //std::cout << filename << std::endl;
    if(filename.empty()){
        std::cout <<"Please enter the correct NRRD file. If you don't understand how to use, enter <-h>" << std::endl;
        return -1;
    }

    unsigned int dimensionality = 0;
    unsigned int number_of_components = 0;
    if(GetNumberOfInputFile(argv, dimensionality, number_of_components) != 0){ //if layers of nrrd file doesn't support
        return -1;
    }
    assert(number_of_components != 0);

    std::vector<std::string>filename_storage;
    filename_storage.reserve(number_of_components);
    filehelper.FilenameGenerator(filename, filename_storage, number_of_components);

    std::vector<std::string>colormap_storage;
    colormap_storage.reserve(number_of_components);
    if(filehelper.ColorMapGenerator(argc, argv, 2, argc, colormap_storage, number_of_components) == -1){
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
