

#include <iostream>

#include "player_class.h"

using namespace std::chrono;
using namespace std;
using namespace cv;

inline void RGB_Gamma_Correction(cv::UMat_Type &src, float Gamma_Correction)
{
  UMat_Type src_squared;
  multiply(src, src, src_squared); // square  255 * too big
  addWeighted(src, 1 - Gamma_Correction, src_squared, .0039 * Gamma_Correction, 0.0, src);
}

Video_Player_With_Processing::Video_Player_With_Processing(void){
  VideoMain.create(IMAGE_ROWS, IMAGE_COLS, CV_8UC3);  

};

void Video_Player_With_Processing::setup(string File_Name, string NameX, bool Movie_Or_Still_In)
{

  Movie_Or_Still = Movie_Or_Still_In;

  if (Movie_Or_Still)
  {
    capMain.open(File_Name);
    ImageWidth = capMain.get(CAP_PROP_FRAME_WIDTH);
    ImageHeight = capMain.get(CAP_PROP_FRAME_HEIGHT);
    ImageDuration = capMain.get(cv::CAP_PROP_FRAME_COUNT);
  }
  else
  {
    VideoMainAlpha = imread(File_Name, IMREAD_UNCHANGED);
    ImageWidth = VideoMainAlpha.cols;
    ImageHeight = VideoMainAlpha.rows;
    ImageDuration = 1;

    // split to BGR and Alpha
    split(VideoMainAlpha, VideoChannels4);

    // note from here down this assumes the transparencey alpha tif mode vs  separate alpha

    // convert alpha channel to floating point 3 channel  0 - 1
    // this takes the 0 -> 255 and inverts it and scales it to  1 -> 0
    VideoChannels4[3].convertTo(Alpha_Channel_F1, CV_32F, -.0039216, 1);
    // creata 3 channel from a 1 channe    // merge the 1st 3 channels of the 4 channel input
    VideoChannels4[0].copyTo(VideoChannels3[0]);
    VideoChannels4[1].copyTo(VideoChannels3[1]);
    VideoChannels4[2].copyTo(VideoChannels3[2]);
    merge(VideoChannels3, 3, VideoMain);
    cv::Mat temp[] = {Alpha_Channel_F1, Alpha_Channel_F1, Alpha_Channel_F1};
    merge(temp, 3, Alpha_Channel_F);

#ifdef UMat_Enable
    Alpha_Channel_F.copyTo(Alpha_Channel_FU);
#else
    Alpha_Channel_FU = Alpha_Channel_F;
#endif

    // merge the 1st 3 channels of the 4 channel input
    VideoChannels4[0].copyTo(VideoChannels3[0]);
    VideoChannels4[1].copyTo(VideoChannels3[1]);
    VideoChannels4[2].copyTo(VideoChannels3[2]);
    merge(VideoChannels3, 3, VideoMain);

    // to view the appha channel
    // Alpha_Channel_F.convertTo(VideoMain, CV_8U, 255, 0);
  }

  // CODING KEY:   F -> float type (vs unsigned char)     U ->  UMat_Type (vs Mat)
  // CREATES NOT NEEDED
  // VideoMain.create(ImageHeight, ImageWidth, CV_8UC(3));
  // VideoMain_U.create(ImageHeight, ImageWidth, CV_8UC(3));
  // VideoMain_FU.create(ImageHeight, ImageWidth, CV_32FC(3));
  // VideoDisplay.create(ImageHeight, ImageWidth, CV_8UC(3));
  // VideoProc_FU.create(ImageHeight, ImageWidth, CV_8UC(3));
  // Color_Difference_FU3.create(ImageHeight, ImageWidth, CV_8UC(3));
  // Y_FU1.create(ImageHeight, ImageWidth, CV_8UC(1));
  // VideoTemp_FU3.create(ImageHeight, ImageWidth, CV_8UC(3));

  player_pause = false;

  display_name = NameX;

  Ones2x2_A = false;
  Ones3x3_A = true;
  Ones4x4_A = false;
  Ones5x5_A = true;
  Ones6x6_A = false;
  Ones7x7_A = false;

  Gain = 1.;
  Black_Level = 0;
  Color_Gain = 2;
  Image_Gamma = 0;

  // figure out where to put this as it's for the building not so much for the picture
  // like color correction
};

void Video_Player_With_Processing::VideoSetup(string File_Name, string NameX)
{

  Movie_Or_Still = 1;

  capMain.open(File_Name);
  ImageWidth = capMain.get(CAP_PROP_FRAME_WIDTH);
  ImageHeight = capMain.get(CAP_PROP_FRAME_HEIGHT);
  ImageDuration = capMain.get(cv::CAP_PROP_FRAME_COUNT);

  // CODING KEY:   F -> float type (vs unsigned char)     U ->  UMat_Type (vs Mat)
  // CREATES NOT NEEDED
  // VideoMain.create(ImageHeight, ImageWidth, CV_8UC(3));
  // VideoMain_U.create(ImageHeight, ImageWidth, CV_8UC(3));
  // VideoMain_FU.create(ImageHeight, ImageWidth, CV_32FC(3));
  // VideoDisplay.create(ImageHeight, ImageWidth, CV_8UC(3));
  // VideoProc_FU.create(ImageHeight, ImageWidth, CV_8UC(3));
  // Color_Difference_FU3.create(ImageHeight, ImageWidth, CV_8UC(3));
  // Y_FU1.create(ImageHeight, ImageWidth, CV_8UC(1));
  // VideoTemp_FU3.create(ImageHeight, ImageWidth, CV_8UC(3));

  Ones_Float_Mat.create(ImageHeight, ImageWidth, CV_32FC(3));
  Ones_Float_Mat = cv::Scalar(1.0, 1.0, 1.0);

  Ones_Float_Mat_U.create(ImageHeight, ImageWidth, CV_32FC(3));
  Ones_Float_Mat_U = cv::Scalar(255.0, 255.0, 255.0);

  player_pause = false;

  display_name = NameX;

  Ones2x2_A = false;
  Ones3x3_A = true;
  Ones4x4_A = false;
  Ones5x5_A = true;
  Ones6x6_A = false;
  Ones7x7_A = false;

  Gain = 1.;
  Black_Level = 0;
  Color_Gain = 2;

  // figure out where to put this as it's for the building not so much for the picture
  // like color correction
  Image_Gamma = 0;
};

void Video_Player_With_Processing::VideoSetup(string File_Name, string NameX, float Gain_In, float Black_Level_In, float Color_Gain_In, float Color_Hue_In, float Gamma_In)
{

  Movie_Or_Still = 1;

  capMain.open(File_Name);
  ImageWidth = capMain.get(CAP_PROP_FRAME_WIDTH);
  ImageHeight = capMain.get(CAP_PROP_FRAME_HEIGHT);
  ImageDuration = capMain.get(cv::CAP_PROP_FRAME_COUNT);

  player_pause = false;

  display_name = NameX;

  Ones2x2_A = false;
  Ones3x3_A = true;
  Ones4x4_A = false;
  Ones5x5_A = true;
  Ones6x6_A = false;
  Ones7x7_A = false;

  Gain = Gain_In;
  Black_Level = Black_Level_In;
  Color_Gain = Color_Gain_In;
  Color_Hue = Color_Hue_In,

  // figure out where to put this as it's for the building not so much for the picture
      // like color correction
      Image_Gamma = Gamma_In;
};

void Video_Player_With_Processing::StillSetup(string File_Name, string NameX)
{

  Movie_Or_Still = 0;

  VideoMainAlpha = imread(File_Name, IMREAD_UNCHANGED);
  ImageWidth = VideoMainAlpha.cols;
  ImageHeight = VideoMainAlpha.rows;
  ImageChannels = VideoMainAlpha.channels();
  ImageDuration = 1;

  Ones_Float_Mat.create(ImageHeight, ImageWidth, CV_32FC(3));
  Ones_Float_Mat = cv::Scalar(1.0, 1.0, 1.0);

  Ones_Float_Mat_U.create(ImageHeight, ImageWidth, CV_32FC(3));
  Ones_Float_Mat_U = cv::Scalar(1.0, 1.0, 1.0);

  Zeros_Float_Mat.create(ImageHeight, ImageWidth, CV_32FC(3));
  Zeros_Float_Mat = cv::Scalar(0.0, 0.0, 0.0);

  Zeros_Float_Mat_U.create(ImageHeight, ImageWidth, CV_32FC(3));
  Zeros_Float_Mat_U = cv::Scalar(0.0, 0.0, 0.0);

  // split to BGR and Alpha
  split(VideoMainAlpha, VideoChannels4);

  // merge the 1st 3 channels of the 4 channel input
  VideoChannels4[0].copyTo(VideoChannels3[0]);
  VideoChannels4[1].copyTo(VideoChannels3[1]);
  VideoChannels4[2].copyTo(VideoChannels3[2]);
  merge(VideoChannels3, 3, VideoMain);

  // note from here down this assumes the transparencey alpha tif mode vs  separate alpha

  // convert alpha channel to floating point 3 channel  0 - 1
  // this takes the 0 -> 255 and inverts it and scales it to  1 -> 0
  VideoChannels4[3].convertTo(Alpha_Channel_F1, CV_32F, -.0039216, 1);
  // creata 3 channel from a 1 channel
  cv::Mat temp[] = {Alpha_Channel_F1, Alpha_Channel_F1, Alpha_Channel_F1};
  merge(temp, 3, Alpha_Channel_F);

#ifdef UMat_Enable
  Alpha_Channel_F.copyTo(Alpha_Channel_FU);
#else
  Alpha_Channel_FU = Alpha_Channel_F;
#endif

  subtract(Ones_Float_Mat, Alpha_Channel_F, Alpha_Channel_Inv_F);

#ifdef UMat_Enable
  Alpha_Channel_Inv_F.copyTo(Alpha_Channel_Inv_FU);
#else
  Alpha_Channel_Inv_FU = Alpha_Channel_Inv_F;
#endif

  // to view the appha channel
  // Alpha_Channel_F.convertTo(VideoMain, CV_8U, 255, 0);

  // CODING KEY:   F -> float type (vs unsigned char)     U ->  UMat_Type (vs Mat)
  // CREATES NOT NEEDED
  // VideoMain.create(ImageHeight, ImageWidth, CV_8UC(3));
  // VideoMain_U.create(ImageHeight, ImageWidth, CV_8UC(3));
  // VideoMain_FU.create(ImageHeight, ImageWidth, CV_32FC(3));
  // VideoDisplay.create(ImageHeight, ImageWidth, CV_8UC(3));
  // VideoProc_FU.create(ImageHeight, ImageWidth, CV_8UC(3));
  // Color_Difference_FU3.create(ImageHeight, ImageWidth, CV_8UC(3));
  // Y_FU1.create(ImageHeight, ImageWidth, CV_8UC(1));
  // VideoTemp_FU3.create(ImageHeight, ImageWidth, CV_8UC(3));

  player_pause = false;

  display_name = NameX;

  Ones2x2_A = false;
  Ones3x3_A = true;
  Ones4x4_A = false;
  Ones5x5_A = true;
  Ones6x6_A = false;
  Ones7x7_A = false;

  Gain = 1.;
  Black_Level = 0;
  Color_Gain = 2;

  // figure out where to put this as it's for the building not so much for the picture
  // like color correction
  Image_Gamma = 0;
};

// rev2 only processes the still images once. To add LIVE color correction I need to detect a change in parameters
void Video_Player_With_Processing::StillSetupWithAlpha(string File_Name, string NameX)
{

  Movie_Or_Still = 0;

  VideoMainAlpha = imread(File_Name, IMREAD_UNCHANGED);
  ImageWidth = VideoMainAlpha.cols;
  ImageHeight = VideoMainAlpha.rows;
  ImageChannels = VideoMainAlpha.channels();
  ImageDuration = 1;

  Ones_Float_Mat.create(ImageHeight, ImageWidth, CV_32FC(3));
  Ones_Float_Mat = cv::Scalar(1.0, 1.0, 1.0);

  Ones_Float_Mat_U.create(ImageHeight, ImageWidth, CV_32FC(3));
  Ones_Float_Mat_U = cv::Scalar(1.0, 1.0, 1.0);

  Zeros_Float_Mat.create(ImageHeight, ImageWidth, CV_32FC(3));
  Zeros_Float_Mat = cv::Scalar(0.0, 0.0, 0.0);

  Zeros_Float_Mat_U.create(ImageHeight, ImageWidth, CV_32FC(3));
  Zeros_Float_Mat_U = cv::Scalar(0.0, 0.0, 0.0);

  // split to BGR and Alpha
  split(VideoMainAlpha, VideoChannels4);

  // merge the 1st 3 channels of the 4 channel input
  VideoChannels4[0].copyTo(VideoChannels3[0]);
  VideoChannels4[1].copyTo(VideoChannels3[1]);
  VideoChannels4[2].copyTo(VideoChannels3[2]);
  merge(VideoChannels3, 3, VideoMain);

  // note from here down this assumes the transparencey alpha tif mode vs  separate alpha

  // convert alpha channel to floating point 3 channel  0 - 1
  // this takes the 0 -> 255 and inverts it and scales it to  1 -> 0
  VideoChannels4[3].convertTo(Alpha_Channel_F1, CV_32F, -.0039216, 1);
  // creata 3 channel from a 1 channel
  cv::Mat temp[] = {Alpha_Channel_F1, Alpha_Channel_F1, Alpha_Channel_F1};
  merge(temp, 3, Alpha_Channel_F);

#ifdef UMat_Enable
  Alpha_Channel_F.copyTo(Alpha_Channel_FU);
#else
  Alpha_Channel_FU = Alpha_Channel_F;
#endif

  subtract(Ones_Float_Mat, Alpha_Channel_F, Alpha_Channel_Inv_F);

#ifdef UMat_Enable
  Alpha_Channel_Inv_F.copyTo(Alpha_Channel_Inv_FU);
#else
  Alpha_Channel_Inv_FU = Alpha_Channel_Inv_F;
#endif

  // to view the appha channel
  // Alpha_Channel_F.convertTo(VideoMain, CV_8U, 255, 0);

  // CODING KEY:   F -> float type (vs unsigned char)     U ->  UMat_Type (vs Mat)
  // CREATES NOT NEEDED
  // VideoMain.create(ImageHeight, ImageWidth, CV_8UC(3));
  // VideoMain_U.create(ImageHeight, ImageWidth, CV_8UC(3));
  // VideoMain_FU.create(ImageHeight, ImageWidth, CV_32FC(3));
  // VideoDisplay.create(ImageHeight, ImageWidth, CV_8UC(3));
  // VideoProc_FU.create(ImageHeight, ImageWidth, CV_8UC(3));
  // Color_Difference_FU3.create(ImageHeight, ImageWidth, CV_8UC(3));
  // Y_FU1.create(ImageHeight, ImageWidth, CV_8UC(1));
  // VideoTemp_FU3.create(ImageHeight, ImageWidth, CV_8UC(3));

  player_pause = false;

  display_name = NameX;

  Ones2x2_A = false;
  Ones3x3_A = true;
  Ones4x4_A = false;
  Ones5x5_A = true;
  Ones6x6_A = false;
  Ones7x7_A = false;

  Gain = 1.;
  Black_Level = 0;
  Color_Gain = 2;

  // figure out where to put this as it's for the building not so much for the picture
  // like color correction
  Image_Gamma = 0;

  Process();
  AlphaProcess();
};

// rev2 only processes the still images once. To add LIVE color correction I need to detect a change in parameters
void Video_Player_With_Processing::StillSetupWithAlpha(string File_Name, string NameX, float Gain_In, float Black_Level_In, float Color_Gain_In, float Color_Hue_In, float Gamma_In)
{

  Movie_Or_Still = 0;

  VideoMainAlpha = imread(File_Name, IMREAD_UNCHANGED);
  ImageWidth = VideoMainAlpha.cols;
  ImageHeight = VideoMainAlpha.rows;
  ImageChannels = VideoMainAlpha.channels();
  ImageDuration = 1;

  Ones_Float_Mat.create(ImageHeight, ImageWidth, CV_32FC(3));
  Ones_Float_Mat = cv::Scalar(1.0, 1.0, 1.0);

  Ones_Float_Mat_U.create(ImageHeight, ImageWidth, CV_32FC(3));
  Ones_Float_Mat_U = cv::Scalar(1.0, 1.0, 1.0);

  Zeros_Float_Mat.create(ImageHeight, ImageWidth, CV_32FC(3));
  Zeros_Float_Mat = cv::Scalar(0.0, 0.0, 0.0);

  Zeros_Float_Mat_U.create(ImageHeight, ImageWidth, CV_32FC(3));
  Zeros_Float_Mat_U = cv::Scalar(0.0, 0.0, 0.0);

  // split to BGR and Alpha
  split(VideoMainAlpha, VideoChannels4);

  // merge the 1st 3 channels of the 4 channel input
  VideoChannels4[0].copyTo(VideoChannels3[0]);
  VideoChannels4[1].copyTo(VideoChannels3[1]);
  VideoChannels4[2].copyTo(VideoChannels3[2]);
  merge(VideoChannels3, 3, VideoMain);

  // note from here down this assumes the transparencey alpha tif mode vs  separate alpha

  // convert alpha channel to floating point 3 channel  0 - 1
  // this takes the 0 -> 255 and inverts it and scales it to  1 -> 0
  VideoChannels4[3].convertTo(Alpha_Channel_F1, CV_32F, -.0039216, 1);
  // creata 3 channel from a 1 channel
  cv::Mat temp[] = {Alpha_Channel_F1, Alpha_Channel_F1, Alpha_Channel_F1};
  merge(temp, 3, Alpha_Channel_F);

#ifdef UMat_Enable
  Alpha_Channel_F.copyTo(Alpha_Channel_FU);
#else
  Alpha_Channel_FU = Alpha_Channel_F;
#endif

  subtract(Ones_Float_Mat, Alpha_Channel_F, Alpha_Channel_Inv_F);

#ifdef UMat_Enable
  Alpha_Channel_Inv_F.copyTo(Alpha_Channel_Inv_FU);
#else
  Alpha_Channel_Inv_FU = Alpha_Channel_Inv_F;
#endif

  // to view the appha channel
  // Alpha_Channel_F.convertTo(VideoMain, CV_8U, 255, 0);

  // CODING KEY:   F -> float type (vs unsigned char)     U ->  UMat_Type (vs Mat)
  // CREATES NOT NEEDED
  // VideoMain.create(ImageHeight, ImageWidth, CV_8UC(3));
  // VideoMain_U.create(ImageHeight, ImageWidth, CV_8UC(3));
  // VideoMain_FU.create(ImageHeight, ImageWidth, CV_32FC(3));
  // VideoDisplay.create(ImageHeight, ImageWidth, CV_8UC(3));
  // VideoProc_FU.create(ImageHeight, ImageWidth, CV_8UC(3));
  // Color_Difference_FU3.create(ImageHeight, ImageWidth, CV_8UC(3));
  // Y_FU1.create(ImageHeight, ImageWidth, CV_8UC(1));
  // VideoTemp_FU3.create(ImageHeight, ImageWidth, CV_8UC(3));

  player_pause = false;

  display_name = NameX;

  Ones2x2_A = false;
  Ones3x3_A = true;
  Ones4x4_A = false;
  Ones5x5_A = true;
  Ones6x6_A = false;
  Ones7x7_A = false;

  Gain = Gain_In;
  Black_Level = Black_Level_In;
  Color_Gain = Color_Gain_In;
  Color_Hue = Color_Hue_In;

  // figure out where to put this as it's for the building not so much for the picture
  // like color correction
  Image_Gamma = Gamma_In;

  Process();
  AlphaProcess();
};

// rev2 only processes the still images once. To add LIVE color correction I need to detect a change in parameters
void Video_Player_With_Processing::AlphaSetupNoProcess(string File_Name, string NameX)
{

  VideoMainAlpha = imread(File_Name, IMREAD_COLOR);

  VideoMainAlpha.convertTo(Alpha_Channel_FU, CV_32F, .0039216);

  display_name = NameX;
};

void Video_Player_With_Processing::Color_Hue_Gain_Set(Pixel &pixel, const float Color_Cos_Mul, const float Color_Sin_Mul)
{
  float temp;

  // this does hue and saturation
  temp = pixel.y * Color_Cos_Mul + pixel.z * Color_Sin_Mul;
  pixel.z = pixel.z * Color_Cos_Mul - pixel.y * Color_Sin_Mul;
  pixel.y = temp;
}

// note figure out how to put these variables in the class
// and the foreach in the class!!!!!!!!!!!!!!!!!!
float coss;
float sinn;

void Color_Gain_Set(Pixel &pixel)
{
  float temp;

  // this does hue and saturation
  temp = pixel.y * coss + pixel.z * sinn;
  pixel.z = pixel.z * coss - pixel.y * sinn;
  pixel.y = temp;
}

struct Operator
{
  void operator()(Pixel &pixel, const int *position) const
  {
    Color_Gain_Set(pixel);
  }
};

void Video_Player_With_Processing::Process(void)
{

  float rz, gz, bz;

  if (Movie_Or_Still)
  {
    if (!player_pause)
    {
      capMain >> VideoIn; // VideoMain; // VideoMain;

      if ( (VideoIn.cols > IMAGE_COLS) || (VideoIn.rows > IMAGE_ROWS) )
      {
        resize(VideoIn, VideoMain, Size(IMAGE_COLS, IMAGE_ROWS), INTER_LINEAR);
      }

      else if(ImageWidth ==  IMAGE_COLS )
        VideoIn.copyTo(VideoMain(cv::Rect(0, 0, VideoIn.cols, VideoIn.rows))); // VideoIn.copyTo(VideoMain);

      else
      {
        VideoIn.copyTo(VideoMain(cv::Rect(0, 0, VideoIn.cols, VideoIn.rows)));
        VideoIn.copyTo(VideoMain(cv::Rect(175, 0, VideoIn.cols, VideoIn.rows)));
        VideoIn.copyTo(VideoMain(cv::Rect(350, 0, VideoIn.cols, VideoIn.rows)));
      }

   //   VideoIn.copyTo(VideoMain);
      // read the video file
      if (capMain.get(cv::CAP_PROP_POS_FRAMES) == ImageDuration)
        capMain.set(cv::CAP_PROP_POS_FRAMES, 0);

      // Change_Seam(img_in_A, 1);  not a UMat_Type function
      Shift_Image_Horizontal(VideoMain, 0);
    }
  }

#ifdef UMat_Enable
  // convert to UMat for faster processing
  VideoMain.copyTo(VideoMain_U);
  // convert to float for more accuracy
  VideoMain_U.convertTo(VideoMain_FU, CV_32FC3);
#else
  VideoMain.convertTo(VideoMain_FU, CV_32FC3);
#endif

bool Filters_On = true;


  // filter choices in-series allowable   ***  NOMINAL 3 & 5 on  ***
  if(Filters_On)
  {  
  if (Ones2x2_A)
    blur(VideoMain_FU, VideoMain_FU, Size(2, 2));
  if (Ones3x3_A)
    blur(VideoMain_FU, VideoMain_FU, Size(3, 3));
  if (Ones4x4_A)
    blur(VideoMain_FU, VideoMain_FU, Size(4, 4));
  if (Ones5x5_A)
    blur(VideoMain_FU, VideoMain_FU, Size(5, 5));
  if (Ones6x6_A)
    blur(VideoMain_FU, VideoMain_FU, Size(6, 6));
  if (Ones7x7_A)
    blur(VideoMain_FU, VideoMain_FU, Size(7, 7));
  }


  TimerLocal.Start_Delay_Timer();

  // gain and black level
  VideoMain_FU.convertTo(VideoMain_FU, -1, Gain, Black_Level * 100);

  // post gain gamma correction
  RGB_Gamma_Correction(VideoMain_FU, Image_Gamma);
  // RGB_Gamma_Correction(VideoMain_FU, 0.0 );

  float Angle = Color_Hue * (3.1416 / 180);
  //  cout << " cosf32(3.1415/4) " << cosf32(Angle) << endl;

  // hue and sat
  float Color_Cos_Mul = Color_Gain * cosf32(Angle);
  float Color_Sin_Mul = Color_Gain * sinf32(Angle);

  // hue and sat  globals for for each versioN
  coss = Color_Gain;
  sinn = Color_Gain;

  // convert to YUV
  cvtColor(VideoMain_FU, VideoTemp_FU3, cv::COLOR_BGR2YUV);




  // MULTIPLE WAYS OF DOING HUE SATURATION!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // depending on file size different versions might be faster !!

  // // use foreach  chroma level and hue
  //    USES GLOABL VARIABLE
  // note figure out how to put these variables in the class
  // and the foreach in the class!!!!!!!!!!!!!!!!!!
  // VideoTemp_FU3.forEach<Pixel>(Operator());
  // cvtColor(VideoTemp_FU3, VideoProc_FU, cv::COLOR_YUV2BGR);

  //    USES GLOABL VARIABLE
  // Using pointer arithmetic  chroma level and hue
  // Get pointer to first pixel
  //   Pixel *pixel = VideoTemp_FU3.ptr<Pixel>(0, 0);
  //   const Pixel *endPixel = pixel + VideoTemp_FU3.cols * VideoTemp_FU3.rows;
  //   // Loop over all pixels
  //   for (; pixel != endPixel; pixel++)
  //   {
  //     Color_Gain_Set(*pixel);
  //   }
  //  cvtColor(VideoTemp_FU3, VideoProc_FU, cv::COLOR_YUV2BGR);

  // //    PART OF CLASS
  // // // Using pointer arithmetic  chroma level and hue
  // // // Get pointer to first pixel
  // Pixel *pixel = VideoTemp_FU3.ptr<Pixel>(0, 0);
  // const Pixel *endPixel = pixel + VideoTemp_FU3.cols * VideoTemp_FU3.rows;
  // // Loop over all pixels
  // for (; pixel != endPixel; pixel++)
  // {
  //   Color_Hue_Gain_Set(*pixel, Color_Cos_Mul, Color_Sin_Mul);
  // }
  // cvtColor(VideoTemp_FU3, VideoProc_FU, cv::COLOR_YUV2BGR);

  // // old way  chroma level ONLY  no HHHHHHHHHHHHUUUUUUUUUUUUUUUUUUUUUUUUUUUEEEEEEEEEEEEEEEEEEEEEe
  // // extract Luminance 1 channel
  // extractChannel(VideoTemp_FU3, Y_FU1, 0);
  // // covert Luminance 1 channel to 3 channels
  // cvtColor(Y_FU1, VideoTemp_FU3, cv::COLOR_GRAY2BGR);
  // // make B-Y  G-Y  R-Y
  // subtract(VideoMain_FU, VideoTemp_FU3, Color_Difference_FU3);
  // // chroma_gain*((B-Y,  G-Y,  R-Y)
  // multiply(Color_Difference_FU3, Color_Gain, Color_Difference_FU3);
  // // chroma_gain*((B-Y,  G-Y,  R-Y)  +  B,G,R  to get back to BGR with color level changed
  // add(VideoTemp_FU3, Color_Difference_FU3, VideoProc_FU);

#ifdef UMat_Enable
  // old way  chroma level ONLY  no HHHHHHHHHHHHUUUUUUUUUUUUUUUUUUUUUUUUUUUEEEEEEEEEEEEEEEEEEEEEe
  // extract Luminance 1 channel
  extractChannel(VideoTemp_FU3, Y_FU1, 0);
  // covert Luminance 1 channel to 3 channels
  cvtColor(Y_FU1, VideoTemp_FU3, cv::COLOR_GRAY2BGR);
  // make B-Y  G-Y  R-Y
  subtract(VideoMain_FU, VideoTemp_FU3, Color_Difference_FU3);
  // chroma_gain*((B-Y,  G-Y,  R-Y)
  multiply(Color_Difference_FU3, Color_Gain, Color_Difference_FU3);
  // chroma_gain*((B-Y,  G-Y,  R-Y)  +  B,G,R  to get back to BGR with color level changed
  add(VideoTemp_FU3, Color_Difference_FU3, VideoProc_FU);
#else
  //    PART OF CLASS  doesnt work with UMAT
  // // Using pointer arithmetic  chroma level and hue
  // // Get pointer to first pixel
  Pixel *pixel = VideoTemp_FU3.ptr<Pixel>(0, 0);
  const Pixel *endPixel = pixel + VideoTemp_FU3.cols * VideoTemp_FU3.rows;
  // Loop over all pixels
  for (; pixel != endPixel; pixel++)
  {
    Color_Hue_Gain_Set(*pixel, Color_Cos_Mul, Color_Sin_Mul);
  }
  cvtColor(VideoTemp_FU3, VideoProc_FU, cv::COLOR_YUV2BGR);
#endif

  TimerLocal.End_Delay_Timer();

#ifdef UMat_Enable
  // convert back to 8 bits unsigned integer  for the display
  VideoProc_FU.convertTo(VideoMain_U, CV_8UC3);
  // make a copy for the display in 8 bit Mat vs UMat
  VideoMain_U.copyTo(VideoDisplay);
#else
  VideoProc_FU.convertTo(VideoDisplay, CV_8UC3);
#endif

}


void Video_Player_With_Processing::AlphaProcess(void)
{

bool Filters_On = true;

#ifdef UMat_Enable
  Alpha_Channel_F.copyTo(Alpha_Channel_FU);
#else
  Alpha_Channel_FU = Alpha_Channel_F;
#endif

  // filter choices in-series allowable   ***  NOMINAL 3 & 5 on  ***
  if(Filters_On)
  {
    if (Ones2x2_A)
      blur(Alpha_Channel_FU, Alpha_Channel_FU, Size(2, 2));
    if (Ones3x3_A)
      blur(Alpha_Channel_FU, Alpha_Channel_FU, Size(3, 3));
    if (Ones4x4_A)
      blur(Alpha_Channel_FU, Alpha_Channel_FU, Size(4, 4));
    if (Ones5x5_A)
      blur(Alpha_Channel_FU, Alpha_Channel_FU, Size(5, 5));
    if (Ones6x6_A)
      blur(Alpha_Channel_FU, Alpha_Channel_FU, Size(6, 6));
    if (Ones7x7_A)
      blur(Alpha_Channel_FU, Alpha_Channel_FU, Size(7, 7));
  }


  Alpha_Channel_Inv_F.copyTo(Alpha_Channel_Inv_FU);
#ifdef UMat_Enable
  Alpha_Channel_Inv_F.copyTo(Alpha_Channel_Inv_FU);
#else
  Alpha_Channel_Inv_FU = Alpha_Channel_Inv_F;
#endif

  // filter choices in-series allowable   ***  NOMINAL 3 & 5 on  ***
  if(Filters_On)
  {
    if (Ones2x2_A)
      blur(Alpha_Channel_Inv_FU, Alpha_Channel_Inv_FU, Size(2, 2));
    if (Ones3x3_A)
      blur(Alpha_Channel_Inv_FU, Alpha_Channel_Inv_FU, Size(3, 3));
    if (Ones4x4_A)
      blur(Alpha_Channel_Inv_FU, Alpha_Channel_Inv_FU, Size(4, 4));
    if (Ones5x5_A)
      blur(Alpha_Channel_Inv_FU, Alpha_Channel_Inv_FU, Size(5, 5));
    if (Ones6x6_A)
      blur(Alpha_Channel_Inv_FU, Alpha_Channel_Inv_FU, Size(6, 6));
    if (Ones7x7_A)
      blur(Alpha_Channel_Inv_FU, Alpha_Channel_Inv_FU, Size(7, 7));
  }



}
