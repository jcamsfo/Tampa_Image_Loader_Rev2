

#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <iostream>
#include <thread>
#include <iomanip>

#include <iostream>
#include <cstdio>
#include <stdio.h>
#include <time.h>
#include <ctime>
#include <unistd.h>
#include <chrono>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <functional> // std::plus
#include <string>
#include <bits/stdc++.h>

#include "KeyValueClient.h"
#include "TestCommon.h"


// #include "defines_Mission_Plaza.h"
#include "defines_Either.h"

#include "player_class.h"
#include "measure2.h"
#include "USB_FTDI.h"
#include "sculpture_class.h"

#include "file_io_2.h"
#include "misc.h"

using namespace std;
using namespace cv;

bool FTDI_On = true;

int main(int argc, char *argv[])
{

  char *TxBuffer;
  char *RxBuffer;

  RxBuffer = new char[RxBuffer_Size];
  TxBuffer = new char[Buffer_W_Gaps_Size_RGBW_Bytes_Extra];

  USB_FTDI_Channel FTX(Buffer_W_Gaps_Size_RGBW_Bytes_Extra_With_Params, RxBuffer_Size);

  Video_Sculpture SC1;

  Prog_Durations Time_Delay[10];
  Prog_Durations Time_Delay_1(30);
  Prog_Durations Time_Delay_2(30);
  Prog_Durations Time_Delay_3(30);
  Prog_Durations Program_Timer(30);
  Prog_Durations Loop_Timer(30);
  Prog_Durations Process_Time(30);

  bool Finished = false;
  bool pauseX = false;
  bool pauseX_old;
  uint16_t last_c;
  int32_t loop = 0;
  float delx;
  uint32_t loopx = 0;
  uint32_t Loop_Cnt = 0;
  uint32_t Bytes_Wrote, Bytes_Read;

  /*******************************************    initialization of GUI code  ******************************************/

  KeyValue key_value_to_send;
  usage("client_test");
  Params params = collect_params(argc, argv);
  KeyValueClient key_value_client;
  key_value_client.connect(params.role, "localhost", params.port, "", "");
  while (key_value_client.connect_result() == ConnectError::PENDING)
  {
    sleep_one_millisecond();
  }
  if (key_value_client.connect_result() != ConnectError::SUCCESS)
  {
    SC1.Enable_Old_Menu = true;
  }
  else
  {
    key_value_client.open_ui_file("../../GUI/MPZ10C.ui");
    // key_value_client.load_key_value_file("../../GUI/defaults10D.json");
    key_value_client.load_key_value_file("../../GUI/defaults_SF_Waves.json");  
    key_value_client.load_key_value_file("../../GUI/defaults10-pad00.json");
    key_value_client.load_key_value_file("../../GUI/defaults10-pad01.json");
    key_value_client.load_key_value_file("../../GUI/defaults10-pad02.json");
    key_value_client.load_key_value_file("../../GUI/defaults10-pad03.json");
    key_value_client.load_key_value_file("../../GUI/defaults10-pad04.json");
    key_value_client.load_key_value_file("../../GUI/defaults10-pad05.json");
    key_value_client.load_key_value_file("../../GUI/defaults10-pad06.json");
    key_value_client.load_key_value_file("../../GUI/defaults10-pad07.json");
    key_value_client.load_key_value_file("../../GUI/defaults10-pad08.json");
    key_value_client.load_key_value_file("../../GUI/defaults10-pad09.json");
    key_value_client.load_key_value_file("../../GUI/defaults10-pad10.json");
    key_value_client.load_key_value_file("../../GUI/defaults10-pad11.json");         
    key_value_client.load_key_value_file("../../GUI/defaults10-pad12.json");        
    key_value_client.load_key_value_file("../../GUI/defaults10-pad13.json");        
    key_value_client.load_key_value_file("../../GUI/defaults10-pad14.json");         
    key_value_client.load_key_value_file("../../GUI/defaults10-pad15.json");                                                 
   }

for(int i=0; i<3000; i++)sleep_one_millisecond();



  /*******************************************    initialization of GUI code DONE  *************************************/

  SC1.Read_Maps();

  while (1)
  {
    Process_Time.Start_Delay_Timer();

    FTX.FTDI_Rx(Bytes_Read);

    // Bytes_Read = 64;

    Bytes_Wrote = 0;
    if (Bytes_Read > 0)
    {
      if (Bytes_Read > 64)
        cout << "********************************   Missed Frame  ******************************* " << Bytes_Read << " Bytes Read " << endl;

      if (FTX.ftStatus == FT_OK)
      {
        /*******************************************    Receive / Send parameters from / to the GUI ******************************************/
        if (!SC1.Enable_Old_Menu) 
        {
          

          string gui_name;
          string gui_value;

          if (key_value_client.connect_result() == ConnectError::SUCCESS || params.role == KeyValueClient::Role::SERVER)
          {
            KeyValue *incoming_key_value = key_value_client.next_received();
            if (incoming_key_value != nullptr)
            {
              cout << "received:" << incoming_key_value->key << " "
                   << incoming_key_value->column << " "
                   << incoming_key_value->row << " "
                   << incoming_key_value->value << endl;

              gui_name = incoming_key_value->key;
              gui_value = incoming_key_value->value;

              // send the new GUI value to Sculpture Class
              SC1.GUI_Receive(gui_name, gui_value);

              delete incoming_key_value;
            }
          }

          Time_Delay_2.Start_Delay_Timer();

          // send single values
          if (SC1.Gui_Name_Sculpture != "")
          {
            cout << " AAA 22  " << SC1.Gui_Name_Sculpture << endl;
            cout << " BBB 22  " << SC1.Gui_Value_Sculpture << endl;
            key_value_to_send.key = SC1.Gui_Name_Sculpture;
            key_value_to_send.value = SC1.Gui_Value_Sculpture;
            key_value_client.send(key_value_to_send);
            SC1.Gui_Name_Sculpture = "";
            SC1.Gui_Value_Sculpture = "";
          }



          // send multiple values
          if (SC1.GUI_Names_Values.size() != 0)
          {
            cout << " AAA 22errrrrrrrrrrrrrrrrrrrrrrrrrr  " << SC1.GUI_Names_Values[0] << endl;

            for (int i = 0; i < SC1.GUI_Names_Values.size(); i += 2)
            {
              key_value_to_send.key = SC1.GUI_Names_Values[i];
              key_value_to_send.value = SC1.GUI_Names_Values[i + 1];
              key_value_client.send(key_value_to_send);
            }
            SC1.GUI_Names_Values.clear();

            // exit(0);
          }


          key_value_to_send.key = "Sun_Gain";
          key_value_to_send.value = to_string((int)(SC1.Sun_Gain * 100));
          key_value_client.send(key_value_to_send);

          key_value_to_send.key = "Final_Gain";
          key_value_to_send.value = to_string((int)(SC1.Final_Output_Gain * 100));
          key_value_client.send(key_value_to_send);
        }

        // if (First_Loop)
        // {
        //   First_Loop = false;
        //   key_value_client.open_ui_file("MPZ3List.ui");
        //   key_value_client.open_ui_file("MPZ5.ui");

        //   key_value_client.load_key_value_file("defaults2.json");

        //   //  key_value_client.disable_widget("Show_Displays");
        //   //   key_value_client.disable_widget("fileBrowser");

        //   // key_value_to_send.key = "tableWidget_2";
        //   // key_value_to_send.value = "hello_there";
        //   // key_value_to_send.row = 3;
        //   // key_value_to_send.column = 2;
        //   // key_value_client.send(key_value_to_send);

        //   // initialization of GUI code from Jonathan DONE
        //   // key_value_client.load_key_value_file("defaults.json");
        // }

        Time_Delay_2.End_Delay_Timer();

        /*******************************************    Receive / Send parameters from / to the GUI DONE *************************************/


        // send all 0s
        if(SC1.End_Application)
        {
          for(int ii=0; ii<Buffer_W_Gaps_Size_RGBW_Bytes_Extra_With_Params; ii++ )
          *(SC1.Sampled_Buffer_RGBW + ii) = 0; 
        }

        FTX.FTDI_Load_TxBuffer((char *)SC1.Sampled_Buffer_RGBW); // Samples_Mapped_To_Sculpture);


        // no threading version  // NUC DELAY = 1.8 ms
        FTX.FTDI_Txx(); // .8 ms
        SC1.Play_All(); // 1.3 ms




        // threading version     // NUC DELAY = 2 ms   has memory leak !!!!!!!!!!!!!!
        // std::thread t1(&Video_Sculpture::Play_All, &SC1);
        // std::thread t2(&USB_FTDI_Channel::FTDI_Txx, &FTX);
        // t1.join();
        // t2.join();

        // NUC DELAY = 9.5 ms NUC
        //Time_Delay_2.Start_Delay_Timer();
        
        SC1.Mixer();
        //Time_Delay_2.End_Delay_Timer();


        // NUC DELAY = .34 ms NUC
        SC1.Multi_Map_Image_To_Sculpture();

        




        // Time_Delay_2.Start_Delay_Timer();
        // NUC DELAY = .67 ms NUC
        SC1.Display();
        // Time_Delay_2.End_Delay_Timer();

        Process_Time.End_Delay_Timer();

        Time_Delay_1.End_Delay_Timer();
        if (SC1.Enable_Time_Display) // loop % 30 == 0)
        {
          cout << "  Loop-cur/avg/max/min:" << std::setprecision(2) << setw(3) << Time_Delay_1.time_delay;
          cout << " " << std::setprecision(2) << setw(2) << Time_Delay_1.time_delay_avg;
          cout << " " << std::setprecision(2) << setw(2) << Time_Delay_1.time_delay_max;
          cout << " " << std::setprecision(2) << setw(2) << Time_Delay_1.time_delay_min;
        }
        Time_Delay_1.Start_Delay_Timer();
        loop++;
        if (SC1.Enable_Time_Display) // loop % 30 == 0)
        {
          cout << "   Process:" << std::setprecision(2) << setw(3) << Process_Time.time_delay;
          cout << " " << std::setprecision(2) << setw(2) << Process_Time.time_delay_avg;
          cout << " " << std::setprecision(2) << setw(2) << Process_Time.time_delay_max;
          cout << " " << std::setprecision(2) << setw(2) << Process_Time.time_delay_min;

          // cout << "    display time: " << Time_Delay_2.time_delay << "  " << Time_Delay_2.time_delay_avg;
          cout << " " << std::setprecision(1) << setw(1) << "  waitkey time: " << Time_Delay_3.time_delay;
          // cout << "    waitkey time: " << Time_Delay_3.time_delay;

          cout << "  frame#: " << loop;

          // cout << "    CCorrection  " << SC1.VP1x.TimerLocal.time_delay << "  " << SC1.VP1x.TimerLocal.time_delay_avg;

          if (Process_Time.time_delay > 25)
            cout << " " << std::setprecision(3) << setw(3) << " ** " << (int)Process_Time.time_delay_max;

         // cout << endl;
        }


        if (SC1.Enable_Time_Display) 
        {
          cout << "   SR: " <<  SC1.Current_Sunrise <<  "  SS: "   << SC1.Current_Sunset << "  CUR: " << SC1.Time_Current_Total ;

                    cout << endl;

        }
        

        Time_Delay_3.Start_Delay_Timer();

        unsigned char c = 255;
        c = (unsigned char)waitKey(1);

        SC1.KeyBoardInput(c, Finished);
        if (Finished)
          break;


        if (SC1.End_Application)
        {
          if( SC1.End_Application_Delay )
            break;

          SC1.End_Application_Delay = SC1.End_Application;
        }


        Time_Delay_3.End_Delay_Timer();
      }
    }

    // not sure why this helps  maybe the FTDI read function being read so many times causes issues
    else
      Delay_Msec(.12); // .25);
  }

  destroyAllWindows();

  return 0;
}
