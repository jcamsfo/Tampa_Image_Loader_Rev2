#ifndef DEFINES_TAMPA_MPZ_H
#define DEFINES_TAMPA_MPZ_H
#pragma once

#include <opencv2/core.hpp>



 // #define UMat_Enable 

#ifdef UMat_Enable
#define UMat_Type UMat
#else
#define UMat_Type Mat
#endif


using namespace std;
using namespace cv;

typedef Vec<uint8_t, 3> Pixel_Type_8b;

typedef Vec<uint16_t, 3> Pixel_Type;

typedef Vec<uint16_t, 3> Pixel_Type_16;

typedef Vec<float, 3> Pixel_Type_F;

// Define a pixel
typedef Point3_<float> Pixel;


// constexpr float Light_Sensor_Upper_Limit    = 10000 ;
// constexpr float Light_Sensor_Lower_Limit    = 3000 ;
// constexpr float Light_Sensor_Gamma_Level    = 1.0 ;
// constexpr float Light_Sensor_Gamma_Power    = 1.5 ;
// constexpr float Light_Sensor_TC             = .01 ;

// constexpr float Light_Sensor_Gain_Upper_Limit   =  1.00 ;  // 90% on in the day time
// constexpr float Light_Sensor_Gain_Lower_Limit   =  .06 ;  // 6% at night

constexpr float Sun_Gain_Upper_Limit   =  1.00 ;  // 90% on in the day time
constexpr float Sun_Gain_Lower_Limit   =  .15 ;  // 6%
constexpr int   Sun_Time_Window_Seconds   =  1200 ;  // 1200 is 20 minutes




// should these be defines or constants ????
// **************************   display Image Locations   ******************************/

// constexpr int Preview_Location_X = 20;
// constexpr int Preview_Location_Y = 20;

// constexpr int Output_Location_X = 20;
// constexpr int Output_Location_Y = 400;

// constexpr int Output_Small_Location_X = 300;
// constexpr int Output_Small_Location_Y = 20;

constexpr int Preview_Location_X = 90;
constexpr int Preview_Location_Y = 50;

constexpr int Output_Location_X = 625;
constexpr int Output_Location_Y = 50;

constexpr int Output_Small_Location_X = 1500;
constexpr int Output_Small_Location_Y = 50;



// **************************   Sculpture Size Definitions Done  ************************/

// should these be defines or constants ????
// **************************   Sculpture Size Definitions   MISSION STREET******/

//constexpr int Sculpture_Size_Pixels = 3840;         //  4 * 16 bit words per pixel
//
//constexpr int Sculpture_Size_RGBW = 15360;              //  16 bit   4 * 11136
//constexpr int Sculpture_Size_RGBW_Bytes = 30720;        //  16 bit   8 * 11136
//
//constexpr int Buffer_W_Gaps_Size_RGBW = 15360;          //  16 bit   4 * 13116
//constexpr int Buffer_W_Gaps_Size_RGBW_Bytes = 30720;   //  16 bit   8 * 13116
//
//constexpr int Buffer_W_Gaps_Size_RGBW_Extra = 15360;        //  16 bit   4 * 13116 with extra
//constexpr int Buffer_W_Gaps_Size_RGBW_Bytes_Extra = 30720; //  16 bit   8 * 13116 with extra
//
//
//// Not Used currently   FOR RGB Sculpture vs RGBW sculpture
//constexpr int Sculpture_Size_RGB = 33408;               //  16 bit   3 * 11136
//constexpr int Sculpture_Size_RGB_Bytes = 66816;         //  16 bit   6 * 11136
//
//constexpr int Buffer_W_Gaps_Size_RGB = 39348;           //  16 bit   3 * 13116
//constexpr int Buffer_W_Gaps_Size_RGB_Bytes = 78696;     //  16 bit   6 * 13116

// **************************   Sculpture Size Definitions Done MISSION STREET******/

// *******************   FTDI USB Size Definitions   MISSION STREET******/
// 1K multiples
// 12288 "pixels"   49152 words  98304 bytesd
//constexpr int TxBuffer_Size_Words = 16384;   //  FTDI needs multiple of 1024
//constexpr int TxBuffer_Size_Bytes = 32768;  //  FTDI needs multiple of 1024
//constexpr int RxBuffer_Size = 1024;
// *******************   FTDI USB Size Definitions  DONE  MISSION STREET******/

//*********************** IMAGE RELATED  ***********************MISSION STREET******/

//// For Info Array Declarations
//constexpr int Max_Num_Of_Image_Files = 100;
//constexpr int Max_File_Name_Length = 25;
//constexpr int Max_Num_Of_Image_Parameters = 50;
//
//
//// constexpr int IMAGE_ROWS = 280;
//// constexpr int IMAGE_COLS = 1024;
//
//constexpr int IMAGE_ROWS = 400;
//constexpr int IMAGE_COLS = 240;
//
//
//constexpr int SAMPLE_ROWS = 80;
//constexpr int SAMPLE_COLS = 48;            // not every row is 256 long

//*********************** IMAGE RELATED DDone MISSION STREET******/

// **************************   Sculpture Size Definitions   ********MISSION PLAZA****/



// ORIGINA MPZ
// constexpr int Sculpture_Size_Pixels = 3600; //  4 * 16 bit words per pixel

// constexpr int Sculpture_Size_RGBW = 14400;       //  16 bit   4 * 3600
// constexpr int Sculpture_Size_RGBW_Bytes = 28800; //  16 bit   8 * 3600

// constexpr int Sculpture_Size_RGB = 10800; //  16 bit   4 * 3600

// // no gaps for headers etc in this work
// constexpr int Buffer_W_Gaps_Size_RGBW = 14400;       //  16 bit   4 * 3600
// constexpr int Buffer_W_Gaps_Size_RGBW_Bytes = 28800; //  16 bit   8 * 3600

// // no extras for headers etc in this work
// constexpr int Buffer_W_Gaps_Size_RGBW_Extra = 14400;       //  16 bit   4 * 3600 with extra
// constexpr int Buffer_W_Gaps_Size_RGBW_Bytes_Extra = 28800; //  16 bit   8 * 3600 with extra

/**********************************/


constexpr int Sculpture_Size_Pixels = 3780; // 3840;         //  4 * 16 bit words per pixel

constexpr int Sculpture_Size_RGBW = 15120; // 15360;              //  16 bit   4 * 11136
constexpr int Sculpture_Size_RGBW_Bytes = 30240; // 30720;        //  16 bit   8 * 11136

constexpr int Sculpture_Size_RGBW_With_Params = 15620; // 15360;              //  16 bit   4 * 11136


constexpr int Sculpture_Size_RGB = 11340;               //  16 bit   3 * 11136

constexpr int Buffer_W_Gaps_Size_RGBW = 15120; // 15360;          //  16 bit   4 * 13116
constexpr int Buffer_W_Gaps_Size_RGBW_Bytes = 30240; // 30720;   //  16 bit   8 * 13116

constexpr int Buffer_W_Gaps_Size_RGBW_Extra = 15120; // 15360;        //  16 bit   4 * 13116 with extra
constexpr int Buffer_W_Gaps_Size_RGBW_Bytes_Extra = 30240; // 30720; //  16 bit   8 * 13116 with extra

constexpr int Buffer_W_Gaps_Size_RGBW_Bytes_Extra_With_Params = 31240; // 30720; //  16 bit   8 * 13116 with extra



constexpr int First_Param_Location = 15500; // 15360;              //  16 bit   4 * 11136



// Not Used currently   FOR RGB Sculpture vs RGBW sculpture
// constexpr int Sculpture_Size_RGB = 33408;               //  16 bit   3 * 11136
// constexpr int Sculpture_Size_RGB_Bytes = 66816;         //  16 bit   6 * 11136

// constexpr int Buffer_W_Gaps_Size_RGB = 39348;           //  16 bit   3 * 13116
// constexpr int Buffer_W_Gaps_Size_RGB_Bytes = 78696;     //  16 bit   6 * 13116

// **************************   Sculpture Size Definitions Done  MISSION PLAZA****/

// *******************   FTDI USB Size Definitions   MISSION PLAZA****/
/// 1K multiples
/// 3600 "pixels"   14400 words  28800 bytes
constexpr int TxBuffer_Size_Words = 16384; //  FTDI needs multiple of 1024
constexpr int TxBuffer_Size_Bytes = 32768; //  FTDI needs multiple of 1024
constexpr int RxBuffer_Size = 1024;
// *******************   FTDI USB Size Definitions  DONE  MISSION PLAZA****/

//*********************** IMAGE RELATED  ********************* MISSION PLAZA****/

// For Info Array Declarations
constexpr int Max_Num_Of_Image_Files = 100;
constexpr int Max_File_Name_Length = 25;
constexpr int Max_Num_Of_Image_Parameters = 50;

// constexpr int IMAGE_ROWS = 280;
// constexpr int IMAGE_COLS = 1024;

// constexpr int IMAGE_ROWS = 250;
// constexpr int IMAGE_COLS = 360;

// constexpr int SAMPLE_ROWS = 50;
// constexpr int SAMPLE_COLS = 72; 

constexpr int IMAGE_ROWS = 180;
constexpr int IMAGE_COLS = 524;

constexpr int SAMPLE_ROWS = 36;
constexpr int SAMPLE_COLS = 105; 



constexpr int V_SAMPLE_SPREAD = 5; // was 4 for Transbay

//*********************** IMAGE RELATED DONE  ****************** MISSION PLAZA****/

// **************************   Other Map Stuff  ******************************/

// NOT USED IN PLAZA PIECE

// constexpr int SAMPLE_LINE_BUFFSIZE = 3000;  // text row length
// constexpr int SAMPLE_MAX_LINE_SIZE = 500;   // elements per row

// constexpr int PANEL_MAP_SIZE    = 3600; // full sculpture
// constexpr int PM_LINE_BUFF_SIZE = 3000;  // csv text row length
// constexpr int PM_MAX_LINE_SIZE  = 2000;  // csv max elements per row

//constexpr int PANEL_MAP_W_GAPS_SIZE     = 11136; // full building
//constexpr int PM_W_GAP_LINE_BUFF_SIZE   = 3000;  // text row length
//constexpr int PM_W_GAP_MAX_LINE_SIZE    = 2000;   // max elements per row

// constexpr int NUM_OF_ENCLOSURES = 32;
// constexpr int NUM_OF_ENCLOSURE_PARAMETERS = 17;
// constexpr int MAX_NUM_OF_PANELS = 6;

// constexpr int Mapped_Enclosure_Address  = 1;
// constexpr int First_Pixel_Location = 2;
// constexpr int Panel1_Offset  = 3;
// constexpr int Green_DAC = 9;
// constexpr int Panel1_LR  = 10;

// **************************   Other Map Stuff Done  **************************/

enum Corrections
{
    Gain_X = 0,
    Black_Level_X = 1,
    Chroma_X = 2,
    Gamma_X = 3,
    Duration_X = 4,
    Mix_Type_X = 5,
    Opacity_X = 6,
    Key_Start_Time_X = 7
};

// Dissolve defines
constexpr float ToA = 1.0;
constexpr float ToB = -1.0;

constexpr int Image_Fading = 0;
constexpr int Image_A = 1;
constexpr int Image_B = 2;

constexpr int Image_Off = 0;
constexpr int Image_Fading_To_Full_On = 1; 
constexpr int Image_Fading_To_Off_From_Partial_On = 7;

constexpr int Image_Fading_To_Partial_On_Negative = 9;
constexpr int Image_Partial_On_Negative = 10;
constexpr int Image_Fading_To_Off_From_Partial_On_Negative = 11;

constexpr int Words_Per_Pixel = 4;

constexpr int RGB = 3;
constexpr int RGBW = 4;






#endif /* DEFINES_H */