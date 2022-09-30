/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2020, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     encmain.cpp
    \brief    Encoder application main
*/

#include <time.h>
#include <iostream>
#include <chrono>
#include <ctime>

#include "EncoderLib/EncLibCommon.h"
#include "EncApp.h"
#include "Utilities/program_options_lite.h"
#include <EncoderLib/PartitionManager.h>
#include <EncoderLib/PartitionPrediction.h>

//! \ingroup EncoderApp
//! \{

static const uint32_t settingNameWidth = 66;
static const uint32_t settingHelpWidth = 84;
static const uint32_t settingValueWidth = 3;
// --------------------------------------------------------------------------------------------------------------------- //
// Extern pointer to store and load the partition + current parameter
PartitionManager * store_partition;
PartitionManager * load_partition;
PartitionParam * param_partition;
PartitionPrediction * predict_partition;
PartitionPrediction * predict_partitionInter;



//fdeep::model *model = static_cast<fdeep::model *>(malloc(sizeof(fdeep::model)));
//std::unique_ptr<fdeep::model> model;

//string folder_model = "MODEL_DIRECTORY_HERE/cnn_model/" ;


float time_cnn = 0;

//macro value printing function

#define PRINT_CONSTANT(NAME, NAME_WIDTH, VALUE_WIDTH) std::cout << std::setw(NAME_WIDTH) << #NAME << " = " << std::setw(VALUE_WIDTH) << NAME << std::endl;

static void printMacroSettings()
{
  if( g_verbosity >= DETAILS )
  {
    std::cout << "Non-environment-variable-controlled macros set as follows: \n" << std::endl;

    //------------------------------------------------

    //setting macros

    PRINT_CONSTANT( RExt__DECODER_DEBUG_BIT_STATISTICS,                         settingNameWidth, settingValueWidth );
    PRINT_CONSTANT( RExt__HIGH_BIT_DEPTH_SUPPORT,                               settingNameWidth, settingValueWidth );
    PRINT_CONSTANT( RExt__HIGH_PRECISION_FORWARD_TRANSFORM,                     settingNameWidth, settingValueWidth );
    PRINT_CONSTANT( ME_ENABLE_ROUNDING_OF_MVS,                                  settingNameWidth, settingValueWidth );

    //------------------------------------------------

    std::cout << std::endl;
  }
}

// ====================================================================================================================
// Main function
// ====================================================================================================================

int main(int argc, char* argv[])
{
  // print information
  fprintf( stdout, "\n" );
  fprintf( stdout, "VVCSoftware: VTM Encoder Version %s ", VTM_VERSION );
  fprintf( stdout, NVM_ONOS );
  fprintf( stdout, NVM_COMPILEDBY );
  fprintf( stdout, NVM_BITS );
#if ENABLE_SIMD_OPT
  std::string SIMD;
  df::program_options_lite::Options opts;
  opts.addOptions()
    ( "SIMD", SIMD, string( "" ), "" )
    ( "c", df::program_options_lite::parseConfigFile, "" );
  df::program_options_lite::SilentReporter err;
  df::program_options_lite::scanArgv( opts, argc, ( const char** ) argv, err );
  fprintf( stdout, "[SIMD=%s] ", read_x86_extension( SIMD ) );
#endif
#if ENABLE_TRACING
  fprintf( stdout, "[ENABLE_TRACING] " );
#endif
#if ENABLE_SPLIT_PARALLELISM
  fprintf( stdout, "[SPLIT_PARALLEL (%d jobs)]", PARL_SPLIT_MAX_NUM_JOBS );
#endif
#if ENABLE_SPLIT_PARALLELISM
  const char* waitPolicy = getenv( "OMP_WAIT_POLICY" );
  const char* maxThLim   = getenv( "OMP_THREAD_LIMIT" );
  fprintf( stdout, waitPolicy ? "[OMP: WAIT_POLICY=%s," : "[OMP: WAIT_POLICY=,", waitPolicy );
  fprintf( stdout, maxThLim   ? "THREAD_LIMIT=%s" : "THREAD_LIMIT=", maxThLim );
  fprintf( stdout, "]" );
#endif
  fprintf( stdout, "\n" );

  std::fstream bitstream;
  EncLibCommon encLibCommon;

  std::vector<EncApp*> pcEncApp(1);
  bool resized = false;
  int layerIdx = 0;



  initROM();
  TComHash::initBlockSizeToIndex();

  char** layerArgv = new char*[argc];

  do
  {
    pcEncApp[layerIdx] = new EncApp( bitstream, &encLibCommon );
    // create application encoder class per layer
    pcEncApp[layerIdx]->create();

    // parse configuration per layer
    try
    {
      int j = 0;
      for( int i = 0; i < argc; i++ )
      {
        if( argv[i][0] == '-' && argv[i][1] == 'l' )
        {
          if (argc <= i + 1)
          {
            THROW("Command line parsing error: missing parameter after -lx\n");
          }
          int numParams = 1; // count how many parameters are consumed
          // check for long parameters, which start with "--"
          const std::string param = argv[i + 1];
          if (param.rfind("--", 0) != 0)
          {
            // only short parameters have a second parameter for the value
            if (argc <= i + 2)
            {
              THROW("Command line parsing error: missing parameter after -lx\n");
            }
            numParams++;
          }
          // check if correct layer index
          if( argv[i][2] == std::to_string( layerIdx ).c_str()[0] )
          {
            layerArgv[j] = argv[i + 1];
            if (numParams > 1)
            {
              layerArgv[j + 1] = argv[i + 2];
            }
            j+= numParams;
          }
          i += numParams;
        }
        else
        {
          layerArgv[j] = argv[i];
          j++;
        }
      }

      if( !pcEncApp[layerIdx]->parseCfg( j, layerArgv ) )
      {
        pcEncApp[layerIdx]->destroy();
        return 1;
      }
    }
    catch( df::program_options_lite::ParseFailure &e )
    {
      std::cerr << "Error parsing option \"" << e.arg << "\" with argument \"" << e.val << "\"." << std::endl;
      return 1;
    }

    pcEncApp[layerIdx]->createLib( layerIdx );

    if( !resized )
    {
      pcEncApp.resize( pcEncApp[layerIdx]->getMaxLayers() );
      resized = true;
    }

    layerIdx++;
  } while( layerIdx < pcEncApp.size() );

  delete[] layerArgv;

  if (layerIdx > 1)
  {
    VPS* vps = pcEncApp[0]->getVPS();
    //check chroma format and bit-depth for dependent layers
    for (uint32_t i = 0; i < layerIdx; i++)
    {
      int curLayerChromaFormatIdc = pcEncApp[i]->getChromaFormatIDC();
      int curLayerBitDepth = pcEncApp[i]->getBitDepth();
      for (uint32_t j = 0; j < layerIdx; j++)
      {
        if (vps->getDirectRefLayerFlag(i, j))
        {
          int refLayerChromaFormatIdcInVPS = pcEncApp[j]->getChromaFormatIDC();
          CHECK_VTM(curLayerChromaFormatIdc != refLayerChromaFormatIdcInVPS, "The chroma formats of the current layer and the reference layer are different");
          int refLayerBitDepthInVPS = pcEncApp[j]->getBitDepth();
          CHECK_VTM(curLayerBitDepth != refLayerBitDepthInVPS, "The bit-depth of the current layer and the reference layer are different");
        }
      }
    }
  }

#if PRINT_MACRO_VALUES
  printMacroSettings();
#endif

  // Get partition param from config file
  param_partition = new PartitionParam(pcEncApp.at(0)->getM_uiCTUSize(), 6*3/*Because I decided to use 6*3 bits to encode MTT in dat file*/, pcEncApp.at(0)->is_writePartition(), pcEncApp.at(0)->is_readPartition(), pcEncApp.at(0)->is_predictPartition(), pcEncApp.at(0)->is_predictPartitionInter());

  //sbelhadj added
  string folder_model = pcEncApp.at(0)->get_modelFolder() ;
  if (folder_model.substr(folder_model.length()-1) != "/") folder_model += "/" ;

  if(param_partition->is_writePartition()){
      //Get name of the input video to create dat file to save partition
      std::size_t posEnd = pcEncApp.at(0)->get_filenameInput().find_last_of("/");
      string filenameFeatures = pcEncApp.at(0)->get_filenameInput().substr(posEnd+1);
      std::size_t pos = filenameFeatures.find(".yuv");
      filenameFeatures = filenameFeatures.substr(0,pos);
      filenameFeatures += "_partition_" + to_string(pcEncApp.at(0)->get_qp()) + ".dat";
      filenameFeatures = pcEncApp.at(0)->get_datFolder() + "/" + filenameFeatures;
      // Create pointer to store partition
      store_partition = new PartitionManager(param_partition, (u_int16_t) pcEncApp.at(0)->get_sourceWidth(),
                                             (u_int16_t) pcEncApp.at(0)->get_sourceHeight(), filenameFeatures, !param_partition->is_writePartition());
      store_partition->store_params();
  }

  if(param_partition->is_readPartition()){
      //Get name of the input video to create dat file to save partition
      std::size_t posEnd = pcEncApp.at(0)->get_filenameInput().find_last_of("/");
      string filenameFeatures = pcEncApp.at(0)->get_filenameInput().substr(posEnd+1);
      std::size_t pos = filenameFeatures.find(".yuv");
      filenameFeatures = filenameFeatures.substr(0,pos);
      filenameFeatures += "_partition_" + to_string(pcEncApp.at(0)->get_qp()) + ".dat";
      filenameFeatures = pcEncApp.at(0)->get_datFolder() + "/" + filenameFeatures;
      // Create pointer to load partition
      load_partition = new PartitionManager(param_partition, (u_int16_t) pcEncApp.at(0)->get_sourceWidth(),
                                            (u_int16_t) pcEncApp.at(0)->get_sourceHeight(), filenameFeatures, param_partition->is_readPartition());
      load_partition->load_params();
  }

  // load the model if we predict intra partition
  if(param_partition->is_predictPartition() ){
    clock_t start = clock();
    //*model = fdeep::load_model(folder_model+"my_model_tech_db_filtered2020-05-13_15-05-43_0.094_0.094.json");

    predict_partition = new PartitionPrediction(folder_model+"cnn_model/intra/model.json", pcEncApp.at(0)->get_qp(), true);

    predict_partition->initializeModels("intra", folder_model);

    time_cnn += ((double) clock() - start) / CLOCKS_PER_SEC;
  }

  // load the model if we predict inter partition
  if(param_partition->is_predictPartitionInter() ){
    clock_t start = clock();
    // *model = fdeep::load_model(folder_model+"my_model_tech_db_filtered2020-05-13_15-05-43_0.094_0.094.json");

//    predict_partitionInter = new PartitionPrediction(folder_model+"inter/20220228_145317_benchmark_mobileNetV2_filteredData.json", pcEncApp.at(0)->get_qp(), false);
    predict_partitionInter = new PartitionPrediction(folder_model+"cnn_model/inter/my_model_inter_3dim_mobilenetv2_batch256_100epoch_dbfilteredaugmented_gooddim_2021-04-19_15-31-43_0.04_0.044.json", pcEncApp.at(0)->get_qp(), false);

    predict_partitionInter->initializeModels("inter", folder_model);

    time_cnn += ((double) clock() - start) / CLOCKS_PER_SEC;
  }


  // starting time
  auto startTime  = std::chrono::steady_clock::now();
  std::time_t startTime2 = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  fprintf(stdout, " started @ %s", std::ctime(&startTime2) );
  clock_t startClock = clock();

  // call encoding function per layer
  bool eos = false;

  while( !eos )
  {
    // read GOP
    bool keepLoop = true;
    while( keepLoop )
    {
      for( auto & encApp : pcEncApp )
      {
#ifndef _DEBUG
        try
        {
#endif
          keepLoop = encApp->encodePrep( eos );
#ifndef _DEBUG
        }
        catch( Exception &e )
        {
          std::cerr << e.what() << std::endl;
          return EXIT_FAILURE;
        }
        catch( const std::bad_alloc &e )
        {
          std::cout << "Memory allocation failed: " << e.what() << std::endl;
          return EXIT_FAILURE;
        }
#endif
      }
    }

    // encode GOP
    keepLoop = true;
    while( keepLoop )
    {
      for( auto & encApp : pcEncApp )
      {
#ifndef _DEBUG
        try
        {
#endif
          keepLoop = encApp->encode();
#ifndef _DEBUG
        }
        catch( Exception &e )
        {
          std::cerr << e.what() << std::endl;
          return EXIT_FAILURE;
        }
        catch( const std::bad_alloc &e )
        {
          std::cout << "Memory allocation failed: " << e.what() << std::endl;
          return EXIT_FAILURE;
        }
#endif
      }
    }
  }
  // ending time
  clock_t endClock = clock();
  auto endTime = std::chrono::steady_clock::now();
  std::time_t endTime2 = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
#if JVET_O0756_CALCULATE_HDRMETRICS
  auto metricTime = pcEncApp[0]->getMetricTime();

  for( int layerIdx = 1; layerIdx < pcEncApp.size(); layerIdx++ )
  {
    metricTime += pcEncApp[layerIdx]->getMetricTime();
  }
  auto totalTime      = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime ).count();
  auto encTime        = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime - metricTime ).count();
  auto metricTimeuser = std::chrono::duration_cast<std::chrono::milliseconds>( metricTime ).count();
#else
  auto encTime = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime).count();
#endif

  for( auto & encApp : pcEncApp )
  {
    encApp->destroyLib();

    // destroy application encoder class per layer
    encApp->destroy();

    delete encApp;
  }

  // destroy ROM
  destroyROM();

  pcEncApp.clear();

  printf( "\n finished @ %s", std::ctime(&endTime2) );

#if JVET_O0756_CALCULATE_HDRMETRICS
  printf(" Encoding Time (Total Time): %12.3f ( %12.3f ) sec. [user] %12.3f ( %12.3f ) sec. [elapsed]\n",
         ((endClock - startClock) * 1.0 / CLOCKS_PER_SEC) - (metricTimeuser/1000.0),
         (endClock - startClock) * 1.0 / CLOCKS_PER_SEC,
         encTime / 1000.0,
         totalTime / 1000.0);
#else
  printf(" Total Time: %12.3f sec. [user] %12.3f sec. [elapsed]\n",
         (endClock - startClock) * 1.0 / CLOCKS_PER_SEC,
         encTime / 1000.0);
#endif

  if(param_partition->is_predictPartition() || param_partition->is_predictPartitionInter()){
    std::cout<<"Time in the CNN + utilization of result: "<<time_cnn<<std::endl;
  }

  // Delete pointer
  delete param_partition;
  delete store_partition;
  delete load_partition;
  delete predict_partition;
  delete predict_partitionInter;

  return 0;
}

//! \}
