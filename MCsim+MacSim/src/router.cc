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
 * File         : router.h 
 * Author       : Jaekyu Lee
 * Date         : 12/7/2011
 * SVN          : $Id: dram.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : Interconnection network
 *********************************************************************************************/


#include <fstream>
#include <cmath>

#include "router.h"
#include "all_knobs.h"
#include "all_stats.h"
#include "utils.h"
#include "debug_macros.h"
#include "assert_macros.h"
#include "memreq_info.h"

#define LOCAL 0
#define LEFT  1
#define RIGHT 2
#define UP    3
#define DOWN  4

#define HEAD 0
#define BODY 1
#define TAIL 2

#define INIT 0
#define IB   1
#define RC   2
#define VCA  3
#define SA   4
#define ST   5
#define LT   6

#define OLDEST_FIRST 0
#define ROUND_ROBIN  1
#define CPU_FIRST    2
#define GPU_FIRST    3

#define GPU_FRIENDLY 0
#define CPU_FRIENDLY 1
#define MIXED        2
#define INTERLEAVED  3



#define DEBUG(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_NOC, ## args)

/////////////////////////////////////////////////////////////////////////////////////////
// m_sw_avail : switch availability
//   allocated to a flit with VCA stage
//   deallocated when this flit finishes LT stage
/////////////////////////////////////////////////////////////////////////////////////////


int g_total_packet = 0;
int g_total_cpu_packet = 0;
int g_total_gpu_packet = 0;


// flit_c constructor
flit_c::flit_c()
{
  init();
}


// flit_c desctructor
flit_c::~flit_c()
{
}


// flit_c initialization
void flit_c::init(void)
{
  m_src       = -1;
  m_dst       = -1;
  m_head      = false;
  m_tail      = false;
  m_req       = NULL;
  m_state     = INIT;
  m_rdy_cycle = ULLONG_MAX;
  m_vc_batch  = false;
  m_sw_batch  = false;
  m_id        = 0;
}


/////////////////////////////////////////////////////////////////////////////////////////


credit_c::credit_c()
{
}

credit_c::~credit_c()
{
}

/////////////////////////////////////////////////////////////////////////////////////////

bool router_sort_fn(router_c *a, router_c *b)
{
  return a->get_id() < b->get_id();
}


/////////////////////////////////////////////////////////////////////////////////////////


// constructor
router_c::router_c(macsim_c* simBase, int type, int id)
  : m_simBase(simBase), m_type(type), m_id(id)
{
  // configurations
  m_num_vc              = *KNOB(KNOB_NUM_VC); 
  m_link_latency        = *KNOB(KNOB_LINK_LATENCY);
  m_arbitration_policy  = *KNOB(KNOB_ARBITRATION_POLICY);
  m_link_width          = *KNOB(KNOB_LINK_WIDTH);
  m_num_vc_cpu          = *KNOB(KNOB_CPU_VC_PARTITION);
  m_next_vc             = 0;

  m_topology = KNOB(KNOB_NOC_TOPOLOGY)->getValue();
  if (m_topology == "ring") {
    m_num_port = 3;
    assert(*KNOB(KNOB_NOC_DIMENSION) == 1);
    assert(m_num_vc >= 2);
  }
  else if (m_topology == "mesh") {
    m_num_port = 5;
    assert(*KNOB(KNOB_NOC_DIMENSION) == 2);
    assert(m_num_vc >= 1);
  }
  else if (m_topology == "simple_noc") {
    m_ds_injection_buffer_max_size = 10;
    m_us_injection_buffer_max_size = 10;

    m_recv_from_us_avail = 0;
    m_recv_from_ds_avail = 0;
    m_send_to_us_avail = 0;
    m_send_to_ds_avail = 0;

    m_max_pending_flits_from_us = 50;
    m_max_pending_flits_from_ds = 50;
    m_pending_flits_from_us = 0;
    m_pending_flits_from_ds = 0;

    m_ds_start = MAX_INT;
    m_us_start = MAX_INT;
  }

  // link setting
  m_opposite_dir[LOCAL] = LOCAL;
  m_opposite_dir[LEFT]  = RIGHT;
  m_opposite_dir[RIGHT] = LEFT;
  m_opposite_dir[UP]    = DOWN;
  m_opposite_dir[DOWN]  = UP;

  m_link[LOCAL] = NULL;
  m_link[LEFT]  = NULL;
  m_link[RIGHT] = NULL;
  m_link[UP]    = NULL;
  m_link[DOWN]  = NULL;

  // memory allocations
  m_req_buffer = new queue<mem_req_s*>;

  m_injection_buffer = new list<mem_req_s*>;
  m_injection_buffer_max_size = 32;

  m_input_buffer    = new list<flit_c*>*[m_num_port];
  m_output_buffer   = new list<flit_c*>*[m_num_port];
  m_route           = new bool***[m_num_port];
  m_route_fixed     = new int*[m_num_port];
  m_output_vc_avail = new bool*[m_num_port];
  m_output_vc_id    = new int*[m_num_port];
  m_output_port_id  = new int*[m_num_port];
  m_credit          = new int*[m_num_port];

  for (int ii = 0; ii < m_num_port; ++ii) {
    m_input_buffer[ii]  = new list<flit_c*>[m_num_vc];
    m_output_buffer[ii] = new list<flit_c*>[m_num_vc];
    m_route_fixed[ii]   = new int[m_num_vc];
    fill_n(m_route_fixed[ii], m_num_vc, -1);
    m_route[ii] = new bool**[m_num_vc];
    for (int jj = 0; jj < m_num_vc; ++jj) {
      m_route[ii][jj] = new bool*[2];
      for (int kk = 0; kk < 2; ++kk) {
        m_route[ii][jj][kk] = new bool[m_num_port];
        //fill_n(m_route[ii][jj][kk], m_num_vc, false); 
        fill_n(m_route[ii][jj][kk], m_num_port, false); 
      }
    }

    m_output_vc_avail[ii] = new bool[m_num_vc];
    fill_n(m_output_vc_avail[ii], m_num_vc, true);

    m_output_vc_id[ii] = new int[m_num_vc];
    fill_n(m_output_vc_id[ii], m_num_vc, -1);

    m_output_port_id[ii] = new int[m_num_vc];
    fill_n(m_output_port_id[ii], m_num_vc, -1);

    m_credit[ii] = new int[m_num_vc];
    fill_n(m_credit[ii], m_num_vc, 10);
  }

  m_buffer_max_size = 10;
  
  // switch
  m_sw_avail  = new Counter[m_num_port];
  fill_n(m_sw_avail, m_num_port, 0);

  m_link_avail = new Counter[m_num_port];
  fill_n(m_link_avail, m_num_port, 0);

  m_pending_credit = new list<credit_c*>;
}


// destructor
router_c::~router_c()
{
  delete m_req_buffer;
  delete m_injection_buffer;
  for (int ii = 0; ii < m_num_port; ++ii) {
    delete[] m_input_buffer[ii];
    delete[] m_output_buffer[ii];
    delete[] m_route[ii];
    delete[] m_route_fixed[ii];
    delete[] m_output_vc_avail[ii];
    delete[] m_output_vc_id[ii];
    delete[] m_credit[ii];
  }
  delete[] m_input_buffer;
  delete[] m_output_buffer;
  delete[] m_route;
  delete[] m_route_fixed;
  delete[] m_output_vc_avail;
  delete[] m_output_vc_id;
  delete[] m_credit;

  delete[] m_sw_avail;
  delete[] m_link_avail;
  
  delete m_pending_credit;
}


// get router id
int router_c::get_id(void)
{
  return m_id;
}


void router_c::set_id(int id)
{
  m_id = id;
}


// set links to other routers
void router_c::set_link(int dir, router_c* link)
{
  m_link[dir] = link;

  int opposite_dir;
  if (dir == LEFT)       opposite_dir = RIGHT;
  else if (dir == RIGHT) opposite_dir = LEFT;
  else if (dir == UP)    opposite_dir = DOWN;
  else if (dir == DOWN)  opposite_dir = UP;
}

void router_c::set_dstream_routers(vector<router_c *>& routers, int ds_start)
{
  for (auto itr = routers.begin(), end = routers.end(); itr != end; ++itr) {
    m_dstream.push_back(*itr);
  }
  m_ds_start = ds_start;
}

void router_c::set_ustream_routers(vector<router_c *>& routers, int us_start)
{
  for (auto itr = routers.begin(), end = routers.end(); itr != end; ++itr) {
    m_ustream.push_back(*itr);
  }
  m_us_start = us_start;
}


// Tick fuction
void router_c::run_a_cycle(void)
{
  if (m_topology == "simple_noc") {
    packet_injection();
  }
  else {
    if (*KNOB(KNOB_IDEAL_NOC)) {
      stage_vca();
      stage_rc();
      local_packet_injection();
    }
    else {
      //  check_starvation();
      process_pending_credit();
      stage_lt();
      stage_st();
      stage_sa();  
      stage_vca();
      stage_rc();
      local_packet_injection();
    }

    // stats
    //  check_channel();
  }
  ++m_cycle;
}


// insert a packet from the network interface (NI)
bool router_c::inject_packet(mem_req_s* req)
{
  if (*KNOB(KNOB_USE_ZERO_LATENCY_NOC)) {
    m_simBase->m_router->insert_into_router_req_buffer(req->m_msg_dst, req);
    return true;
  }
  else
  {
    if (m_topology == "simple_noc") {
      assert(req->m_msg_src == m_id);
      if (req->m_msg_src < req->m_msg_dst) {
        if (m_ds_injection_buffer.size() < m_ds_injection_buffer_max_size) {
          m_ds_injection_buffer.push(req);

          DEBUG("cycle:%-10lld node:%d [IP] req_id:%d src:%d dst:%d insert success\n",
              m_cycle, m_id, req->m_id, req->m_msg_src, req->m_msg_dst);

          return true;
        }
        else {

          DEBUG("cycle:%-10lld node:%d [IP] req_id:%d src:%d dst:%d insert failure\n",
              m_cycle, m_id, req->m_id, req->m_msg_src, req->m_msg_dst);

          return false;
        }
      }
      else if (req->m_msg_src > req->m_msg_dst) {
        if (m_us_injection_buffer.size() < m_us_injection_buffer_max_size) {
          m_us_injection_buffer.push(req);

          DEBUG("cycle:%-10lld node:%d [IP] req_id:%d src:%d dst:%d insert success\n",
              m_cycle, m_id, req->m_id, req->m_msg_src, req->m_msg_dst);

          return true;
        }
        else {

          DEBUG("cycle:%-10lld node:%d [IP] req_id:%d src:%d dst:%d insert failure\n",
              m_cycle, m_id, req->m_id, req->m_msg_src, req->m_msg_dst);

          return false;
        }
      }
      else {
        assert(0);
      }

    }
    else {
      if (m_injection_buffer->size() < m_injection_buffer_max_size) {
        m_injection_buffer->push_back(req);
        return true;
      }
    }
  }

  return false;
}

bool router_c::try_packet_insert(int src, mem_req_s *req)
{
  int num_flits = 1;
  if ((req->m_msg_type == NOC_NEW_WITH_DATA) || (req->m_msg_type == NOC_FILL)) {
    num_flits += (req->m_size / m_link_width);
  }

  if (src < m_id) {
    if (m_recv_from_us_avail <= m_cycle && 
        ((m_pending_flits_from_us + num_flits) <= m_max_pending_flits_from_us)) {
      m_packets_from_us.push(req);
      req->m_rdy_cycle = m_cycle + num_flits;

      m_pending_flits_from_us += num_flits;
      m_recv_from_us_avail = m_cycle + num_flits;

      DEBUG("cycle:%-10lld node:%d [TPI] req_id:%d src:%d dst:%d num_flits:%d rdy_cycle:%-10lld flit send success\n",
          m_cycle, m_id, req->m_id, req->m_msg_src, req->m_msg_dst, num_flits, req->m_rdy_cycle);

      return true;
    }
    else {
      DEBUG("cycle:%-10lld node:%d [TPI] req_id:%d src:%d dst:%d flit send failure\n",
          m_cycle, m_id, req->m_id, req->m_msg_src, req->m_msg_dst);

      return false;
    }
  }
  else if (src > m_id) {
    if (m_recv_from_ds_avail <= m_cycle && 
        ((m_pending_flits_from_ds + num_flits) <= m_max_pending_flits_from_ds)) {
      m_packets_from_ds.push(req);
      req->m_rdy_cycle = m_cycle + num_flits;

      m_pending_flits_from_ds += num_flits;
      m_recv_from_ds_avail = m_cycle + num_flits;

      DEBUG("cycle:%-10lld node:%d [TPI] req_id:%d src:%d dst:%d num_flits:%d rdy_cycle:%-10lld flit send success\n",
          m_cycle, m_id, req->m_id, req->m_msg_src, req->m_msg_dst, num_flits, req->m_rdy_cycle);

      return true;
    }
    else {
      DEBUG("cycle:%-10lld node:%d [TPI] req_id:%d src:%d dst:%d flit send failure\n",
          m_cycle, m_id, req->m_id, req->m_msg_src, req->m_msg_dst);

      return false;
    }
  }
  else {
    assert(0);
  }
}
void router_c::packet_injection(void)
{
  mem_req_s *req = NULL;

  if (m_ds_injection_buffer.size()) {
    req = m_ds_injection_buffer.front();
    if (m_dstream[req->m_msg_dst - m_ds_start]->try_packet_insert(m_id, req)) {
        DEBUG("cycle:%-10lld node:%d [PI] req_id:%d src:%d dst:%d dsr_id:%d try packet insert success\n",
            m_cycle, m_id, req->m_id, req->m_msg_src, req->m_msg_dst, (req->m_msg_dst - m_ds_start));
      m_ds_injection_buffer.pop();
    }
    else {
        DEBUG("cycle:%-10lld node:%d [PI] req_id:%d src:%d dst:%d dsr_id:%d try packet insert failure\n",
            m_cycle, m_id, req->m_id, req->m_msg_src, req->m_msg_dst, (req->m_msg_dst - m_ds_start));
    }
  }

  if (m_us_injection_buffer.size()) {
    req = m_us_injection_buffer.front();
    if (m_ustream[req->m_msg_dst - m_us_start]->try_packet_insert(m_id, req)) {
        DEBUG("cycle:%-10lld node:%d [PI] req_id:%d src:%d dst:%d dsr_id:%d try packet insert success\n",
            m_cycle, m_id, req->m_id, req->m_msg_src, req->m_msg_dst, (req->m_msg_dst - m_us_start));
      m_us_injection_buffer.pop();
    }
    else {
        DEBUG("cycle:%-10lld node:%d [PI] req_id:%d src:%d dst:%d dsr_id:%d try packet insert failure\n",
            m_cycle, m_id, req->m_id, req->m_msg_src, req->m_msg_dst, (req->m_msg_dst - m_us_start));
    }
  }
}

// insert a packet from the injection buffer
void router_c::local_packet_injection(void)
{
  while (1) {
    if (m_injection_buffer->empty())
      break;

    bool req_inserted = false;
#ifdef GPU_VALIDATION
    int last_tried_vc = -1;
#endif
    for (int ii = 0; ii < m_num_vc; ++ii) {
      // check buffer availability to insert a new request
      bool cpu_queue = false;
      mem_req_s* req;
      if (m_injection_buffer->empty())
        continue;
      req = m_injection_buffer->front();
      cpu_queue = false;

      assert(req);
      int num_flit = 1;
      if ((req->m_msg_type == NOC_NEW_WITH_DATA) || (req->m_msg_type == NOC_FILL)) 
        num_flit += req->m_size / m_link_width; 

      if (*KNOB(KNOB_IDEAL_NOC))
        num_flit = 1;


#ifdef GPU_VALIDATION
      last_tried_vc = (m_next_vc + ii) % m_num_vc;
      if (m_input_buffer[0][(m_next_vc + ii) % m_num_vc].size() + num_flit <= m_buffer_max_size) {
#else
      if (m_input_buffer[0][ii].size() + num_flit <= m_buffer_max_size) {
#endif
        // flit generation and insert into the buffer
        STAT_EVENT(TOTAL_PACKET_CPU + req->m_ptx);
        req->m_noc_cycle = m_cycle;

        // stat handling
        ++g_total_packet;
        if (req->m_ptx) {
          ++g_total_gpu_packet;
          STAT_EVENT(NOC_AVG_ACTIVE_PACKET_BASE_GPU);
          STAT_EVENT_N(NOC_AVG_ACTIVE_PACKET_GPU, g_total_gpu_packet);
        }
        else {
          ++g_total_cpu_packet;
          STAT_EVENT(NOC_AVG_ACTIVE_PACKET_BASE_CPU);
          STAT_EVENT_N(NOC_AVG_ACTIVE_PACKET_CPU, g_total_cpu_packet);
        }

        STAT_EVENT(NOC_AVG_ACTIVE_PACKET_BASE);
        STAT_EVENT_N(NOC_AVG_ACTIVE_PACKET, g_total_packet);

        // packet generation
        for (int jj = 0; jj < num_flit; ++jj) {
          flit_c* new_flit = m_flit_pool->acquire_entry();
          new_flit->m_req = req;
          new_flit->m_src = req->m_msg_src;
          new_flit->m_dst = req->m_msg_dst;

          if (jj == 0) new_flit->m_head = true;
          else new_flit->m_head = false;

          if (jj == num_flit-1) new_flit->m_tail = true;
          else new_flit->m_tail = false;

          new_flit->m_state      = IB;
          new_flit->m_timestamp  = m_cycle;
          new_flit->m_rdy_cycle  = m_cycle;
          new_flit->m_id         = jj;
          new_flit->m_dir        = -1;

          // insert all flits to input_buffer[LOCAL][vc]
#ifdef GPU_VALIDATION
          m_input_buffer[0][(m_next_vc + ii) % m_num_vc].push_back(new_flit);
#else
          m_input_buffer[0][ii].push_back(new_flit);
#endif
        }

        // pop a request from the injection queue
        m_injection_buffer->pop_front();
        req_inserted = true;

        DEBUG("cycle:%-10lld node:%d [IB] req_id:%d src:%d dst:%d\n",
            m_cycle, m_id, req->m_id, req->m_msg_src, req->m_msg_dst);

        break; //why this break?
      }
    }

#ifdef GPU_VALIDATION
    if (last_tried_vc != -1 && *KNOB(KNOB_USE_RR_FOR_NOC_INSERTION)) {
      m_next_vc = (last_tried_vc + 1) % m_num_vc;
    }
#endif

    // Nothing was scheduled. Stop inserting!
    if (!req_inserted)
      break;
  }
}


// RC (Route Calculation) stage
void router_c::stage_rc(void)
{
  if (m_topology == "ring")
    rc_ring();
  else if (m_topology == "mesh")
    rc_mesh();
  else
    assert(0);
}


void router_c::rc_mesh(void)
{
  for (int port = 0; port < m_num_port; ++port) {
    for (int vc = 0; vc < m_num_vc; ++vc) {
      if (m_input_buffer[port][vc].empty())
        continue;

      flit_c* flit = m_input_buffer[port][vc].front();
      if (flit->m_head == true && flit->m_state == IB && flit->m_rdy_cycle <= m_cycle) {
        fill_n(m_route[port][vc][0], m_num_port, false);
        fill_n(m_route[port][vc][1], m_num_port, false);
        // local
        if (flit->m_dst == m_id) {
          m_route[port][vc][0][LOCAL] = true;
          m_route[port][vc][1][LOCAL] = true;
        }
        else {
          int width = sqrt(m_total_router);
          int x_src = m_id % width;
          int y_src = m_id / width;
          int x_dst = flit->m_dst % width;
          int y_dst = flit->m_dst / width;

          // adaptive routing
          if (x_src > x_dst)
            m_route[port][vc][0][LEFT] = true;
          else if (x_src < x_dst)
            m_route[port][vc][0][RIGHT] = true;

          if (y_src > y_dst) 
            m_route[port][vc][0][UP] = true;
          else if (y_src < y_dst)
            m_route[port][vc][0][DOWN] = true;

          // escape routing
          if (x_src > x_dst)
            m_route[port][vc][1][LEFT] = true;
          else if (x_src < x_dst)
            m_route[port][vc][1][RIGHT] = true;
          else if (y_src > y_dst) 
            m_route[port][vc][1][UP] = true;
          else if (y_src < y_dst)
            m_route[port][vc][1][DOWN] = true;
        }
        flit->m_state = RC;
        DEBUG("cycle:%-10lld node:%d [RC] req_id:%d flit_id:%d src:%d dst:%d ip:%d vc:%d\n",
            m_cycle, m_id, flit->m_req->m_id, flit->m_id, flit->m_req->m_msg_src, 
            flit->m_req->m_msg_dst, port, vc);
      }
    }
  }
}


void router_c::rc_ring(void)
{
  for (int port = 0; port < m_num_port; ++port) {
    for (int vc = 0; vc < m_num_vc; ++vc) {
      if (m_route_fixed[port][vc] != -1 || m_input_buffer[port][vc].empty())
        continue;

      flit_c* flit = m_input_buffer[port][vc].front();
      if (flit->m_head == true && flit->m_state == IB && flit->m_rdy_cycle <= m_cycle) {
        fill_n(m_route[port][vc][0], m_num_port, false);
        // ------------------------------
        // shortest distance routing
        // ------------------------------
        if (flit->m_dst == m_id) {
          m_route[port][vc][0][LOCAL] = true;
        }
        else if (flit->m_dir != -1) {
          m_route[port][vc][0][flit->m_dir] = true;
        }
        else {
          // ------------------------------
          // Ring
          // ------------------------------
          int left;
          int right;
          if (m_id > flit->m_dst) {
            left = m_id - flit->m_dst;
            right = flit->m_dst - m_id + m_total_router;
          }
          else {
            left = m_id - flit->m_dst + m_total_router;
            right = flit->m_dst - m_id;
          }

          if (left < right) {
            m_route[port][vc][0][LEFT] = true;
            flit->m_dir = LEFT;
          }
          else {
            m_route[port][vc][0][RIGHT] = true;
            flit->m_dir = RIGHT;
          }
        }

        flit->m_state = RC;
        DEBUG("cycle:%-10lld node:%d [RC] req_id:%d flit_id:%d src:%d dst:%d ip:%d vc:%d\n",
            m_cycle, m_id, flit->m_req->m_id, flit->m_id, flit->m_req->m_msg_src, 
            flit->m_req->m_msg_dst, port, vc);
      }
    }
  }
}


void router_c::stage_vca(void)
{
  if (*KNOB(KNOB_IDEAL_NOC)) {
    for (int ip = 0; ip < m_num_port; ++ip) {
      for (int ivc = 0; ivc < m_num_vc; ++ivc) {
        if (m_input_buffer[ip][ivc].empty()) {
          continue;
        }

        flit_c* flit = m_input_buffer[ip][ivc].front();
        if (flit->m_state != RC || flit->m_timestamp + *KNOB(KNOB_IDEAL_NOC_LATENCY) > m_cycle) {
          continue;
        }
        // insert to next router
        int port = -1;
        for (int ii = 0; ii < m_num_port; ++ii) {
          if (m_route[ip][ivc][0][ii]) {
            port = ii;
            break;
          }
        }
        assert(port != -1);

        if (port != LOCAL) {
          m_link[port]->insert_packet(flit, m_opposite_dir[port], ivc);
          flit->m_rdy_cycle = m_cycle + 1;
        }

        // delete flit in the buffer
        m_input_buffer[ip][ivc].pop_front();

        // free 1) switch, 2) ivc_avail[ip][ivc], 3) rc, 4) vc
        if (flit->m_tail) {
          if (port == LOCAL) {
            m_req_buffer->push(flit->m_req);
          }
        }

        if (port == LOCAL) {
          flit->init();
          m_flit_pool->release_entry(flit);
        }
      }
    }
    return ;
  }

  // VCA (Virtual Channel Allocation) stage
  // for each output port
  for (int oport = 0; oport < m_num_port; ++oport) {
    for (int ovc = 0; ovc < m_num_vc; ++ovc) {
      // if there is available vc in the output port
      if (m_output_vc_avail[oport][ovc]) { 
        int iport, ivc;
        stage_vca_pick_winner(oport, ovc, iport, ivc);
        if (iport != -1) {
          flit_c* flit = m_input_buffer[iport][ivc].front();
          flit->m_state = VCA;
          
          m_output_port_id[iport][ivc]  = oport;
          m_output_vc_id[iport][ivc]    = ovc;
          m_output_vc_avail[oport][ovc] = false;
          m_route_fixed[iport][ivc]     = oport;

          DEBUG("cycle:%-10lld node:%d [VA] req_id:%d flit_id:%d src:%d dst:%d ip:%d ic:%d "
              "op:%d oc:%d ptx:%d\n",
              m_cycle, m_id, flit->m_req->m_id, flit->m_id, flit->m_req->m_msg_src, 
              flit->m_req->m_msg_dst, iport, ivc, m_route_fixed[iport][ivc], ovc, 
              flit->m_req->m_ptx);
        }
      }
    }
  }
}


void router_c::stage_vca_pick_winner(int oport, int ovc, int& iport, int& ivc)
{
  int rc_index = ovc / (m_num_vc - 1);
  if (m_topology == "ring")
    rc_index = 0;


  // Oldest-first arbitration
  if (m_arbitration_policy == OLDEST_FIRST) {
    // search all input ports for the winner
    Counter oldest_timestamp = ULLONG_MAX;
    iport = -1;
    for (int ii = 0; ii < m_num_port; ++ii) {
      if (ii == oport)
        continue;

      for (int jj = 0; jj < m_num_vc; ++jj) {
        if (m_input_buffer[ii][jj].empty() || !m_route[ii][jj][rc_index][oport])
          continue;

        flit_c* flit = m_input_buffer[ii][jj].front();


        // DUATO's deadlock prevention protocol
        if (m_topology == "ring") {
          if (ovc < m_num_vc - 2) {
            // do nothing
          }
          else if (ovc == m_num_vc - 2) {
            if (m_id > flit->m_dst)
              continue;
          }
          else if (ovc == m_num_vc - 1) {
            if (m_id < flit->m_dst)
              continue;
          }
        }


        // header && RC stage && oldest
        if (flit->m_head == true && flit->m_state == RC && flit->m_timestamp < oldest_timestamp) {
          oldest_timestamp = flit->m_timestamp;
          iport = ii;
          ivc   = jj;
        }
      }
    }
  }
#if 0
  // round-robin
  else if (1) {
    // search all input ports for the winner
    Counter oldest_timestamp = ULLONG_MAX;
    iport = -1;
    for (int rr = 0; rr < m_num_port*m_num_vc; ++rr) {
      int ii = (CYCLE + rr) % m_num_port;
      int jj = ((CYCLE + rr) % (m_num_port*m_num_vc)) / m_num_port;
      
      if (ii == oport)
        continue;

      if (m_input_buffer[ii][jj].empty() || !m_route[ii][jj][rc_index][oport])
        continue;

      flit_c* flit = m_input_buffer[ii][jj].front();

      // header && RC stage && oldest
      if (flit->m_head == true && flit->m_state == RC && flit->m_timestamp < oldest_timestamp) {
        oldest_timestamp = flit->m_timestamp;
        iport = ii;
        ivc   = jj;
      }
    }
  }
#endif
  else
    assert(0);
}


int router_c::get_ovc_occupancy(int port, bool type)
{
  int search_begin = 0;
  int search_end = m_num_vc;

  if (m_enable_vc_partition) {
    if (type == false) {
      search_end = m_num_vc_cpu;
    }
    else {
      search_begin = m_num_vc_cpu;
    }
  }

  int count = 0;

  for (int vc = search_begin; vc < search_end; ++vc) 
    if (m_output_vc_avail[port][vc])
      ++count;

  return count;
}


// SA (Switch Allocation) stage
void router_c::stage_sa(void)
{
  for (int op = 0; op < m_num_port; ++op) {
    if (m_sw_avail[op] <= m_cycle) {
      int ip, ivc;
      stage_sa_pick_winner(op, ip, ivc, 0); 

      if (ip != -1) {
        flit_c* flit = m_input_buffer[ip][ivc].front();
        flit->m_state = SA;
        
        m_sw_avail[op] = m_cycle + 1;
        
        DEBUG("cycle:%-10lld node:%d [SA] req_id:%d flit_id:%d src:%d dst:%d ip:%d ic:%d op:%d oc:%d route:%d port:%d\n",
            m_cycle, m_id, flit->m_req->m_id, flit->m_id, flit->m_req->m_msg_src, 
            flit->m_req->m_msg_dst, ip, ivc, m_route_fixed[ip][ivc], m_output_vc_id[ip][ivc], 
            m_route_fixed[ip][ivc], m_output_port_id[ip][ivc]);
      }
    }
  }
}


void router_c::stage_sa_pick_winner(int op, int& ip, int& ivc, int sw_id)
{
  if (m_arbitration_policy == OLDEST_FIRST) {
    Counter oldest_timestamp = ULLONG_MAX;
    ip = -1;
    for (int ii = 0; ii < m_num_port; ++ii) {
      if (ii == op)
        continue;

      for (int jj = 0; jj < m_num_vc; ++jj) {
        // find a flit that acquires a vc in current output port
        if (m_route_fixed[ii][jj] == op && 
            m_output_port_id[ii][jj] == op && 
            !m_input_buffer[ii][jj].empty()) {
          flit_c* flit = m_input_buffer[ii][jj].front();

          // VCA & Header or IB & Body/Tail
          if (((flit->m_head && flit->m_state == VCA) ||
               (!flit->m_head && flit->m_state == IB)) && 
              flit->m_timestamp < oldest_timestamp) {
            oldest_timestamp = flit->m_timestamp;
            ip = ii;
            ivc = jj;
          }
        }
      }
    }
  }
}


// ST-stage
void router_c::stage_st(void)
{
  for (int op = 0; op < m_num_port; ++op) {
    int ip = -1;
    int ivc = -1;
    for (int ii = 0; ii < m_num_port; ++ii) {
      if (ii == op)
        continue;

      for (int jj = 0; jj < m_num_vc; ++jj) {
        if (m_route_fixed[ii][jj] == op && 
            m_output_port_id[ii][jj] == op && 
            !m_input_buffer[ii][jj].empty()) {
          flit_c* flit = m_input_buffer[ii][jj].front();
          if (flit->m_state == SA) {
            ip = ii;
            ivc = jj;
            break;
          }
        }
      }
    }

    if (ip != -1) {

      flit_c* flit = m_input_buffer[ip][ivc].front();
      flit->m_state = ST;

      // insert a flit to the output buffer 
      int ovc = m_output_vc_id[ip][ivc];
      m_output_buffer[op][ovc].push_back(flit);

      // pop a flit from the input buffer
      m_input_buffer[ip][ivc].pop_front();

      // all flits traversed, so need to free a input vc
      if (flit->m_tail) {
        m_route_fixed[ip][ivc]    = -1;
        m_output_vc_id[ip][ivc]   = -1;
        m_output_port_id[ip][ivc] = -1;
      }

      // send a credit back to previous router
      if (ip != LOCAL) {
        credit_c* credit = m_credit_pool->acquire_entry();
        credit->m_port = m_opposite_dir[ip];
        credit->m_vc = ivc;
        credit->m_rdy_cycle = m_cycle + 1;
        m_link[ip]->insert_credit(credit);
      }


      DEBUG("cycle:%-10lld node:%d [ST] req_id:%d flit_id:%d src:%d dst:%d ip:%d ic:%d port:%d ovc:%d\n",
          m_cycle, m_id, flit->m_req->m_id, flit->m_id, flit->m_req->m_msg_src, 
          flit->m_req->m_msg_dst, ip, ivc, m_output_port_id[ip][ivc], ovc);
    }
  }
}

void router_c::stage_lt(void)
{
  for (int port = 0; port < m_num_port; ++port) {
    if (m_link_avail[port] > m_cycle) 
#ifdef GPU_VALIDATION
      continue; //break;
#else
      break;
#endif
    Counter oldest_cycle = ULLONG_MAX;
    flit_c* f = NULL;
    int vc = -1;
    for (int ii_d = 0; ii_d < m_num_vc; ++ii_d) {
      int ii = (ii_d + m_cycle) % m_num_vc;
      if (m_output_buffer[port][ii].empty() || m_credit[port][ii] == 0)
        continue;

      flit_c* flit = m_output_buffer[port][ii].front();
      assert(flit->m_state == ST);
      
      if (flit->m_timestamp < oldest_cycle) {
        oldest_cycle = flit->m_timestamp;
        vc           = ii;
        f            = flit;
        break;
      }
    }

    if (vc != -1) {
      // insert to next router
      if (port != LOCAL) {
        DEBUG("cycle:%-10lld node:%d [LT] req_id:%d flit_id:%d src:%d dst:%d port:%d vc:%d\n",
            m_cycle, m_id, f->m_req->m_id, f->m_id, f->m_req->m_msg_src, 
            f->m_req->m_msg_dst, port, vc);

        m_link[port]->insert_packet(f, m_opposite_dir[port], vc);
        f->m_rdy_cycle = m_cycle + m_link_latency + 1;
        m_link_avail[port] = m_cycle + m_link_latency; // link busy
        STAT_EVENT(NOC_LINK_ACTIVE);
      }

      // delete flit in the buffer
      m_output_buffer[port][vc].pop_front();

      if (port != LOCAL)
        --m_credit[port][vc];

      // free 1) switch, 2) ivc_avail[ip][ivc], 3) rc, 4) vc
      if (f->m_tail) {
        STAT_EVENT(NOC_AVG_WAIT_IN_ROUTER_BASE);
        STAT_EVENT_N(NOC_AVG_WAIT_IN_ROUTER, m_cycle - f->m_timestamp);

        STAT_EVENT(NOC_AVG_WAIT_IN_ROUTER_BASE_CPU + m_type);
        STAT_EVENT_N(NOC_AVG_WAIT_IN_ROUTER_CPU + m_type, m_cycle - f->m_timestamp);

        m_output_vc_avail[port][vc] = true;


        if (port == LOCAL) {
          --g_total_packet;
          if (f->m_req->m_ptx) {
            --g_total_gpu_packet;
          }
          else {
            --g_total_cpu_packet;
          }
          m_req_buffer->push(f->m_req);
          DEBUG("cycle:%-10lld node:%d [TT] req_id:%d flit_id:%d src:%d dst:%d vc:%d\n",
              m_cycle, m_id, f->m_req->m_id, f->m_id, f->m_req->m_msg_src, 
              f->m_req->m_msg_dst, vc);

          STAT_EVENT(NOC_AVG_LATENCY_BASE);
          STAT_EVENT_N(NOC_AVG_LATENCY, m_cycle - f->m_req->m_noc_cycle);

          STAT_EVENT(NOC_AVG_LATENCY_BASE_CPU + f->m_req->m_ptx);
          STAT_EVENT_N(NOC_AVG_LATENCY_CPU + f->m_req->m_ptx, m_cycle - f->m_req->m_noc_cycle);
        }
      }

      if (port == LOCAL) {
        f->init();
        m_flit_pool->release_entry(f);
      }
    }
  }
}


void router_c::check_channel(void)
{
}


void router_c::insert_packet(flit_c* flit, int port, int vc)
{
  // IB (Input Buffering) stage
  if (flit->m_head)
    DEBUG("cycle:%-10lld node:%d [IB] req_id:%d src:%d dst:%d ip:%d vc:%d\n",
        m_cycle, m_id, flit->m_req->m_id, flit->m_req->m_msg_src, flit->m_req->m_msg_dst, port, vc);

  m_input_buffer[port][vc].push_back(flit);
  flit->m_state = IB;
}


void router_c::insert_credit(credit_c* credit)
{
  m_pending_credit->push_back(credit);
}


void router_c::process_pending_credit(void)
{
  if (m_pending_credit->empty())
    return ;

  auto I = m_pending_credit->begin();
  auto E = m_pending_credit->end();
  do {
    credit_c* credit = (*I++);
    if (credit->m_rdy_cycle <= CYCLE) {
      ++m_credit[credit->m_port][credit->m_vc];
      m_pending_credit->remove(credit);
      m_credit_pool->release_entry(credit);
    }
  } while (I != E);
}


mem_req_s* router_c::receive_req(int dir)
{
  if (m_topology == "simple_noc") {
    mem_req_s *req;

    if (dir == 0) {
      if (m_packets_from_us.size()) {
        req = m_packets_from_us.front();
        if (req->m_rdy_cycle <= m_cycle) {
          return req;
        }
      }
      return NULL;
    }
    else if (dir == 1) {
      if (m_packets_from_ds.size()) {
        req = m_packets_from_ds.front();
        if (req->m_rdy_cycle <= m_cycle) {
          return req;
        }
      }
      return NULL;
    }
    else {
      assert(0);
    }

  }
  else {
    if (m_req_buffer->empty())
      return NULL;
    else
      return m_req_buffer->front();
  }
}


void router_c::pop_req(int dir)
{
  if (m_topology == "simple_noc") {
    mem_req_s *req;

    if (dir == 0) {
      assert(m_packets_from_us.size());
      req = m_packets_from_us.front();
      m_packets_from_us.pop();

      m_pending_flits_from_us -= 1 + (((req->m_msg_type == NOC_NEW_WITH_DATA) || (req->m_msg_type == NOC_FILL))? (req->m_size / m_link_width) : 0);
      }
    else if (dir == 1) {
      assert(m_packets_from_ds.size());
      req = m_packets_from_ds.front();
      m_packets_from_ds.pop();

      m_pending_flits_from_ds -= 1 + (((req->m_msg_type == NOC_NEW_WITH_DATA) || (req->m_msg_type == NOC_FILL)) ? (req->m_size / m_link_width) : 0);
    }
    else {
      assert(0);
    }
  }
  else {
    m_req_buffer->pop();
  }
}


void router_c::init(int total_router, int* total_packet, pool_c<flit_c>* flit_pool, pool_c<credit_c>* credit_pool)
{
  m_total_router   = total_router;
  m_total_packet   = total_packet;
  m_flit_pool      = flit_pool;
  m_credit_pool    = credit_pool;
}


void router_c::print(ofstream& out)
{
}


void router_c::print_link_info(void)
{
  if (m_topology == "ring") {
    cout << "Ring configuration\n";
    cout << m_id << " <-> ";
    router_c* current = m_link[RIGHT];
    do {
      cout << current->get_id() << " <-> ";
      current = current->m_link[RIGHT];
    } while (current != this);
    cout << "\n";
  }
  else if (m_topology == "mesh") {
    cout << "Mesh configuration\n";
    cout << m_id << " <-> ";
    int dir = RIGHT; 
    router_c* current = m_link[dir];
    while (1) {
      cout << current->get_id() << " <-> ";
      if (current->m_link[dir] == NULL) {
        cout << "\n";
        dir = m_opposite_dir[dir];
        if (current->m_link[DOWN] == NULL) {
          break;
        }
        current = current->m_link[DOWN];
      }
      else {
        current = current->m_link[dir];
      }
    }
  }
}


void router_c::check_starvation(void)
{
  return ;
}

/////////////////////////////////////////////////////////////////////////////////////////


router_wrapper_c::router_wrapper_c(macsim_c* simBase)
  : m_simBase(simBase)
{
  m_total_packet = 0;
  m_num_router = 0;

  m_flit_pool  = new pool_c<flit_c>(100, "flit");
  m_credit_pool = new pool_c<credit_c>(100, "credit");
}

router_wrapper_c::~router_wrapper_c()
{
  delete m_flit_pool;
  delete m_credit_pool;
}


// run one cycle for all routers
void router_wrapper_c::run_a_cycle(void)
{
  // randomized tick function
  int index = CYCLE % m_num_router;
  for (int ii = index; ii < index + m_num_router; ++ii) {
    m_router[ii % m_num_router]->run_a_cycle();
  }
  ++m_cycle;
}


// create a router
router_c* router_wrapper_c::create_router(int type)
{
  report("router:" << m_num_router << " type:" << type << " created");

  router_c* new_router = new router_c(m_simBase, type, m_num_router++);
  m_router.push_back(new_router);

  return new_router;
}


void router_wrapper_c::init(void)
{
  // topology - setting router links
  m_topology = KNOB(KNOB_NOC_TOPOLOGY)->getValue();
  if (m_topology == "mesh") {
    int width = sqrt(m_num_router);
    if ((width * width) != m_num_router) {
      for (; m_num_router < (width+1)*(width+1) ; ++m_num_router) {
        report("router:" << m_num_router << " type:dummy created");

        router_c* new_router = new router_c(m_simBase, 0, m_num_router);
        m_router.push_back(new_router);
      }
    }
  }

  for (int ii = 0; ii < m_num_router; ++ii) {
    m_router[ii]->init(m_num_router, &g_total_packet, m_flit_pool, m_credit_pool);
  }

  if (m_topology == "ring")
    init_ring();
  else if (m_topology == "mesh")
    init_mesh();
  else if (m_topology == "simple_noc")
    init_simple_topology();
  else
    assert(0);
}


// 2D mesh initialization
// TODO (jaekyu, 5-9-2012)
// improve mapping
void router_wrapper_c::init_mesh(void)
{
  int* mapping = new int[m_num_router];
  stringstream sstr;
  int num_large_core = *KNOB(KNOB_NUM_SIM_LARGE_CORES);
  int num_small_core = *KNOB(KNOB_NUM_SIM_SMALL_CORES);

  int count = 0;
  
  for (int ii = 0; ii < num_large_core; ++ii) {
    mapping[count++] = ii;
  } 

  if (num_large_core == 0) {
    for (int ii = num_large_core; ii < num_large_core+num_small_core; ++ii) {
      mapping[count++] = ii;
    }
  }

  int start_index = num_large_core + num_small_core;
  int end_index = start_index + *KNOB(KNOB_NUM_L3) + *KNOB(KNOB_DRAM_NUM_MC); 
  for (int ii = start_index; ii < end_index; ++ii) {
    mapping[count++] = ii;
  }

  if (num_large_core != 0) {
    for (int ii = num_large_core; ii < num_large_core+num_small_core; ++ii) {
      mapping[count++] = ii;
    }
  }

  for (int ii = end_index; ii < m_num_router; ++ii)
    mapping[count++] = ii;

  int width = sqrt(m_num_router);
  for (int ii = 0; ii < m_num_router; ++ii) {
    if (ii / width > 0)  // north link
      m_router[mapping[ii]]->set_link(UP, m_router[mapping[ii-width]]);

    if (ii / width < (width - 1))  // south link
      m_router[mapping[ii]]->set_link(DOWN, m_router[mapping[ii+width]]);

    if (ii % width != 0)  // west link
      m_router[mapping[ii]]->set_link(LEFT, m_router[mapping[ii-1]]);

    if (ii % width != (width - 1))  // east link
      m_router[mapping[ii]]->set_link(RIGHT, m_router[mapping[ii+1]]);

    m_router[mapping[ii]]->set_id(ii);
  }

  m_router[0]->print_link_info();
}


// ring topology initialization
void router_wrapper_c::init_ring(void)
{
  string mapping;
  stringstream sstr;
  for (int ii = 0; ii < m_num_router; ++ii) {
    sstr << ii << ",";
  }
  sstr >> mapping;
  mapping = mapping.substr(0, mapping.length()-1);

  int search_pos = 0;
  int pos;
  vector<int> map_func;
  while (1) {
    pos = mapping.find(',', search_pos);
    if (pos == string::npos) {
      string sub = mapping.substr(search_pos);
      map_func.push_back(atoi(sub.c_str()));
      break;
    }

    string sub = mapping.substr(search_pos, pos - search_pos);
    map_func.push_back(atoi(sub.c_str()));
    search_pos = pos + 1;
  }

  assert(m_num_router == map_func.size());

  for (int ii = 0; ii < m_num_router; ++ii) {
    m_router[map_func[ii]]->set_link(LEFT,  m_router[map_func[(ii-1+m_num_router)%m_num_router]]);
    m_router[map_func[ii]]->set_link(RIGHT, m_router[map_func[(ii+1)%m_num_router]]);
    m_router[map_func[ii]]->set_id(ii);
  }

  m_router[0]->print_link_info();
}

void router_wrapper_c::init_simple_topology(void)
{
  for(auto itr = m_router.begin(), end = m_router.end(); itr != end; ++itr) {
    if ((*itr)->m_type == GPU_ROUTER) {
      m_cores.push_back(*itr);
    }
    else if ((*itr)->m_type == L3_ROUTER) {
      m_caches.push_back(*itr);
    }
    else if ((*itr)->m_type == MC_ROUTER) {
      m_mcs.push_back(*itr);
    }
    else {
      assert(0);
    }
  }

  /*
  int id = 0;
  for (auto itr = m_cores.begin(), end = m_cores.end(); itr != end; ++itr) {
    (*itr)->set_id(id++);
  }

  for (auto itr = m_caches.begin(), end = m_caches.end(); itr != end; ++itr) {
    (*itr)->set_id(id++);
  }

  for (auto itr = m_mcs.begin(), end = m_mcs.end(); itr != end; ++itr) {
    (*itr)->set_id(id++);
  }
  */

  for (auto itr = m_cores.begin(), end = m_cores.end(); itr != end; ++itr) {
    (*itr)->set_dstream_routers(m_caches, m_cores.size());
  }

  for (auto itr = m_caches.begin(), end = m_caches.end(); itr != end; ++itr) {
    (*itr)->set_dstream_routers(m_mcs, m_cores.size() + m_caches.size());
    (*itr)->set_ustream_routers(m_cores, 0);
  }

  for (auto itr = m_mcs.begin(), end = m_mcs.end(); itr != end; ++itr) {
    (*itr)->set_ustream_routers(m_caches, m_cores.size());
  }
}


void router_wrapper_c::print(void)
{
  ofstream out("router.out");
  out << "total_packet:" << m_total_packet << "\n";
  for (int ii = 0; ii < m_num_router; ++ii) {
    m_router[ii]->print(out);
  }
}

void router_wrapper_c::insert_into_router_req_buffer(int router_id, mem_req_s *req)
{
  m_router[router_id]->m_req_buffer->push(req);
}


/////////////////////////////////////////////////////////////////////////////////////////
