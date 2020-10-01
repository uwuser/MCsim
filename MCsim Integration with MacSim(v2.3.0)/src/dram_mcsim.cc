/*
Copyright (c) <2012>, <Georgia Institute of Technology> All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted 
provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions 
and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list of 
conditions and the following disclaimer in the documentation and/or other materials provided 
with the distribution.

Neither the name of the <Georgia Institue of Technology> nor the names of its contributors 
may be used to endorse or promote products derived from this software without specific prior 
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.
*/


/**********************************************************************************************
 * File         : dram_mcsim.cc 
 * Author       : Reza Mirosanlou
 * Date         : 2/18/2013
 * SVN          : $Id: dram.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : MCsim interface
 *********************************************************************************************/


#ifdef MCSIM
#include "../src/MCsim/src/MCsim.h"

#include "dram_mcsim.h"
#include "assert_macros.h"
#include "debug_macros.h"
#include "bug_detector.h"
#include "memory.h"

#include "all_knobs.h"
#include "statistics.h"


#undef DEBUG
#define DEBUG(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_DRAM, ## args)

using namespace MCsim;

/** FIXME
 * How to handle redundant requests
 * Fix .ini file and output directories
 */





//unsigned int num_cores = *m_simBase->m_knobs->KNOB_NUM_SIM_CORES; // To get the number of cores

dram_mcsim_c::dram_mcsim_c(macsim_c *simBase) : dram_c(simBase)
{
  //cout<<"set the controller"<<endl;
  m_simBase = simBase;
  unsigned int m_num_core = *m_simBase->m_knobs->KNOB_NUM_SIM_CORES;
  m_output_buffer = new list<mem_req_s *>;
  m_tmp_output_buffer = new list<mem_req_s *>;
  m_pending_request = new list<mem_req_s *>;
  m_mcsim = getMemorySystemInstance(
      m_num_core,
      "../src/MCsim/system/FRFCFS_BACKEND/FRFCFS_BACKEND.ini", //this should be parameterized 
      "DDR3", 
      "1600H",  
      "2Gb_x8",
	1,
	1); // 2048*4 = 4 ranks

  // this only makes sense when MC frequency is the same as that of CPU
  m_mcsim->setCPUClockSpeed(*KNOB(KNOB_CLOCK_MC) * 1e9);

  TransactionCompleteCB *read_cb = new Callback<dram_mcsim_c, void, unsigned, uint64_t, uint64_t>(this, &dram_mcsim_c::read_callback);
	TransactionCompleteCB *write_cb = new Callback<dram_mcsim_c, void, unsigned, uint64_t, uint64_t>(this, &dram_mcsim_c::write_callback);

  m_mcsim->RegisterCallbacks(read_cb, write_cb);
}

dram_mcsim_c::~dram_mcsim_c()
{
  delete m_output_buffer;
  delete m_tmp_output_buffer;
  delete m_pending_request;
  delete m_mcsim;
}

void dram_mcsim_c::print_req(void)
{
}

void dram_mcsim_c::init(int id)
{
  m_id = id;
}

void dram_mcsim_c::run_a_cycle(bool temp)
{
  send();
  m_mcsim->update();
  receive();
  ++m_cycle;
}

void dram_mcsim_c::read_callback(unsigned id, uint64_t address, uint64_t clock_cycle)
{
 // cout<<"read callback"<<endl;
  // find requests with this address
  auto I = m_pending_request->begin();
  auto E = m_pending_request->end();
  while (I != E) {
    mem_req_s* req = (*I);
    ++I;

    if (req->m_addr == address) {
      if (*KNOB(KNOB_DRAM_ADDITIONAL_LATENCY)) {
        req->m_rdy_cycle = m_cycle + *KNOB(KNOB_DRAM_ADDITIONAL_LATENCY);
        m_tmp_output_buffer->push_back(req);
      } else
        m_output_buffer->push_back(req);
      m_pending_request->remove(req);
    }
  }
}


void dram_mcsim_c::write_callback(unsigned id, uint64_t address, uint64_t clock_cycle)
{
 // cout<<"write callback  "<<address<<"  id  "<<id<<endl;
  // find requests with this address
  auto I = m_pending_request->begin();
  auto E = m_pending_request->end();
  while (I != E) {
    mem_req_s* req = (*I);
    ++I;

    if (req->m_addr == address) {
     // cout<<"retire here"<<endl;
      // in case of WB, retire requests here
      MEMORY->free_req(req->m_core_id, req);
      m_pending_request->remove(req);
    }
  }
}


void dram_mcsim_c::receive(void)
{
  mem_req_s* req = NETWORK->receive(MEM_MC, m_id);
  if (!req)
    return;
  //bool WRITE = (req->m_type == MRT_WB||req->m_type == MRT_DSTORE)? true:false;
  //bool READ = !WRITE; // MCsim is expecting read to be 1 and write to be 0
  //cout<<"add the request"<<endl;
  if (m_mcsim->addRequest(req->m_core_id, static_cast<uint64_t>(req->m_addr), req->m_type == MRT_WB, 64))  {
    STAT_EVENT(TOTAL_DRAM);
    m_pending_request->push_back(req);
    NETWORK->receive_pop(MEM_MC, m_id);
    if (*KNOB(KNOB_BUG_DETECTOR_ENABLE)) {
      m_simBase->m_bug_detector->deallocate_noc(req);
    }
  }
}


void dram_mcsim_c::send(void)
{
  vector<mem_req_s*> temp_list;

  for (auto I = m_tmp_output_buffer->begin(), E = m_tmp_output_buffer->end(); I != E; ++I) {
    mem_req_s* req = *I;
    if (req->m_rdy_cycle <= m_cycle) {
      temp_list.push_back(req);
      m_output_buffer->push_back(req);
    } else {
      break;
    }
  }

  for (auto itr = temp_list.begin(), end = temp_list.end(); itr != end; ++itr) {
    m_tmp_output_buffer->remove((*itr));
  }

  for (auto I = m_output_buffer->begin(), E = m_output_buffer->end(); I != E; ++I) {
    mem_req_s* req = (*I);
    req->m_msg_type = NOC_FILL;
    bool insert_packet = NETWORK->send(req, MEM_MC, m_id, MEM_LLC, req->m_cache_id[MEM_LLC]);
    
    if (!insert_packet) {
      DEBUG("MC[%d] req:%d addr:0x%llx type:%s noc busy\n", 
          m_id, req->m_id, req->m_addr, mem_req_c::mem_req_type_name[req->m_type]);
      break;
    }

    temp_list.push_back(req);
    if (*KNOB(KNOB_BUG_DETECTOR_ENABLE) && *KNOB(KNOB_ENABLE_NEW_NOC)) {
      m_simBase->m_bug_detector->allocate_noc(req);
    }
  }


  for (auto I = temp_list.begin(), E = temp_list.end(); I != E; ++I) {
    m_output_buffer->remove((*I));
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////
// wrapper functions to allocate dram controller object

dram_c* mcsim_controller(macsim_c* simBase)
{
  return new dram_mcsim_c(simBase);
}

#endif


