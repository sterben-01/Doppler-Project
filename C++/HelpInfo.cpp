#include <iostream>
#include <string>


struct HelpInfo{
    static void printHelperInfo(){
        std::cout <<"--------------------------------- Command line format: ----------------------------------------\n" << std::endl;
        std::cout <<"Command line format will be: <Original NRRD file name>, <ColorOption>..." << std::endl;
        std::cout <<"Example: test_img.nrrd R G B\n" << std::endl;
        std::cout <<"--------------------------------- Colormap Parameter: -----------------------------------------\n" << std::endl;
        std::cout <<"Grayscale, Jet, Hot, Cool, \"Black to red\", \"Black to green\", \"Black to blue\"" << std::endl;
        std::cout <<"Spring, Summer, Autumn, Winter, Copper, HSV, \"Blue to white to red\"" << std::endl;
        std::cout <<"\"Red to white to blue\", \"Speed image (blue to black to white)\"" << std::endl;
        std::cout <<"\"Speed image (semi-transparent overlay)\", \"Level set image\"\n" << std::endl;
        std::cout <<"--------------------------------- Colormap Parameter Abbrviation: ------------------------------\n" << std::endl;
        std::cout <<"In order to reduce the command of the user input, some commonly used color also mapped to abbrviation:"<<std::endl;
        std::cout <<" Grayscale -> Gr \n Hot -> H \n Cool -> C \n Black to red -> R \n Black to greed -> G \n Black to blue -> B\n" <<std::endl;
        std::cout <<"--------------------------------- Splitted file name format: -----------------------------------\n" << std::endl;
        std::cout <<"Splitted file name format: <Original NRRD file name>_splitted_<No.>" << std::endl;
        std::cout <<"Example:" << std::endl;
        std::cout <<"test_img_splitted_1.nrrd, test_img_splitted_2.nrrd ... \n" << std::endl;
        std::cout <<"--------------------------------- Generated ITKSNAP workspace file name format: ----------------\n" << std::endl;
        std::cout <<"Generated ITKSNAP workspace file name will be the <Original NRRD file name>.itksnap" << std::endl;
        std::cout <<"If the samename ITKSNAP workspace file is already existed, It will prompt user overwrite it or use new name" << std::endl;
        std::cout <<"New name will be <Original NRRD file name>_1.itksnap\n" << std::endl;
        std::cout <<"--------------------------------- SPECIAL NOTATION: --------------------------------------------\n" << std::endl;
        std::cout <<"1. NOTE: This program currently only support 2~6 layers(channels) NRRD file." << std::endl;
        std::cout <<"2. NOTE: For generated ITKSNAP workspace file, It will only prompt once. If the new filename is also occupied by other file, it will replace that file.\n" << std::endl;
    }
};