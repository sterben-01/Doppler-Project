import sys
import SimpleITK as sitk
import nibabel as nib
from glob import glob
import os
import argparse

Colormaps = {1: "Grayscale",
             2: "Jet",
             3: "Hot",
             4: "Cool",
             5: "Black to red",
             6: "Black to green",
             7: "Black to blue",
             8: "Spring",
             9: "Summer",
             10: "Autumn",
             11: "Winter",
             12: "Copper",
             13: "HSV",
             14: "Blue to white to red",
             15: "Red to white to blue",
             16: "Speed image (blue to black to white)",
             17: "Speed image (semi-transparent overlay)",
             18: "Level set image"}


def check_image_shape(img_name):
    img = nib.load(img_name).get_fdata()
    return img.shape


def separate_image(img_name, save_name):
    img = nib.load(img_name)
    # print(img)
    img_data = img.get_fdata()
    # print(img_data.shape)
    if len(img_data.shape) > 3:
        for i in range(img_data.shape[-1]):
            channel = img_data[:, :, :, :, i]
            output = nib.Nifti1Image(channel, img.affine)
            nib.save(output, save_name + "_channel_" + str(i) + '.nii.gz')


def convert_nrrd_to_nifti(file_name, save_name, postfix):
    if postfix == 'nrrd':
        img = sitk.ReadImage(file_name)
        sitk.WriteImage(img, save_name + ".nii.gz")
    img_name = save_name + ".nii.gz"
    return img_name


def check_colormap(img_name, colormap):
    if colormap and len(colormap) != check_image_shape(img_name)[-1]:
        print("The number of colormap option does not match the number of layers of the actual image!")
        os.system("python3 " + sys.argv[0] + " -h")
        exit()


def check_script_name(name):
    if os.path.exists(name):
        reply = input("This script already exist under this directory. "
                      "Do you want to overwrite this script? (yes/no): ")
        if reply not in ["yes", "Yes", "YES", "y", "Y"]:
            script_name = input("Please give another valid name for generated script: ")
            while os.path.exists(script_name):
                script_name = input("Invalid name! Please give another valid name for generated script: ")
            return script_name
    return name


def generate_script(img_name, script_name, colormap=None):
    cmd_string = "itksnap-wt -layers-set-main " + img_name + "_channel_0.nii.gz"
    if colormap:
        cmd_string += " -props-set-colormap '" + Colormaps[int(colormap[0])] + "'"
    channel_num = 1
    for name in sorted(glob(img_name + "_channel_[1-9]*")):
        cmd_string += " -layers-add-anat " + name
        if colormap:
            cmd_string += " -props-set-colormap '" + Colormaps[int(colormap[channel_num])] + "'"
        channel_num += 1
    cmd_string += " -o " + script_name
    # print(cmd_string)
    os.system(cmd_string)


def parseCmdLineArgs():
    parser = argparse.ArgumentParser(formatter_class=argparse.RawDescriptionHelpFormatter,
                                     description='Help Page of Doppler Image Processing Tool')
    parser.add_argument("Image", help="Image to process (in .nrrd format or .nii.gz format")
    parser.add_argument("Script", help="ITKSnap script name to generate")
    parser.add_argument("-c", "--colormap", nargs='*',
                        help='''Colormap specified for each layer. 
                             Allowed values are: 
                             1: "Grayscale",
                             2: "Jet",
                             3: "Hot",
                             4: "Cool",
                             5: "Black to red",
                             6: "Black to green",
                             7: "Black to blue",
                             8: "Spring",
                             9: "Summer",
                             10: "Autumn",
                             11: "Winter",
                             12: "Copper",
                             13: "HSV",
                             14: "Blue to white to red",
                             15: "Red to white to blue",
                             16: "Speed image (blue to black to white)",
                             17: "Speed image (semi-transparent overlay)",
                             18: "Level set image"
                             (e.g. "-c 1 2 3" indicate that Grayscale for the first layer, 
                             Jet for the second layer and Hot for the third layer)''')
    args = parser.parse_args()
    return args


def main():
    parsed_args = parseCmdLineArgs()
    file_name = parsed_args.Image
    script_name = parsed_args.Script
    colormap = parsed_args.colormap
    save_name, postfix = file_name.split(".")

    img_name = convert_nrrd_to_nifti(file_name, save_name, postfix)
    check_colormap(img_name, colormap)
    script_name = check_script_name(script_name)
    separate_image(img_name, save_name)
    generate_script(save_name, script_name, colormap)


if __name__ == '__main__':
    main()
