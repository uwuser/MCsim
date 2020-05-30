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


#ifndef ROUTER_H
#define ROUTER_H

#include <vector>
#include <unordered_map>
#include <queue>
#include <list>

#include "macsim.h"


#define CPU_ROUTER 0
#define GPU_ROUTER 1
#define L3_ROUTER 2
#define MC_ROUTER 3


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Flit data structure
///////////////////////////////////////////////////////////////////////////////////////////////
class flit_c
{
  public:
    /**
     * Constructor
     */
    flit_c();

    /**
     * Destructor
     */
    ~flit_c();

    /**
     * Initialize function
     */
    void init(void);

  public:
    int        m_src; /**< msg source */
    int        m_dst; /**< msg destination */
    int        m_dir; /**< direction */
    bool       m_head; /**< header flit? */
    bool       m_tail; /**< tail flit? */
    mem_req_s* m_req; /**< pointer to the memory request */
    int        m_state; /**< current state */
    Counter    m_timestamp; /**< timestamp */
    Counter    m_rdy_cycle; /**< ready cycle */
    Counter    m_rc_changed; /**< route changed? */
    bool       m_vc_batch; /**< VCA batch */
    bool       m_sw_batch; /**< SWA batch */
    int        m_id; /**< flit id */
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Credit class - for credit-based flow control
///////////////////////////////////////////////////////////////////////////////////////////////
class credit_c
{
  public:
    /**
     * Constructor
     */
    credit_c();

    /**
     * Destructor
     */
    ~credit_c();

  public:
    Counter m_rdy_cycle; /**< credit ready cycle */
    int m_port; /**< credit port */
    int m_vc; /**< credit vc */
};


bool router_sort_fn(router_c *a, router_c *b);

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Router class
///////////////////////////////////////////////////////////////////////////////////////////////
class router_c
{
  friend class router_wrapper_c;

  public:
    /**
     * Constructor
     */
    router_c(macsim_c* simBase, int type, int id);

    /**
     * Destructor
     */
    ~router_c();

    /**
     * Run a cycle (tick) function
     */
    void run_a_cycle();

    /**
     * Inject a packet into the network
     */
    bool inject_packet(mem_req_s* req);

    /**
     * Insert a packet into the next router
     */
    void insert_packet(flit_c* flit, int ip, int ivc);

    /**
     * Get router id
     */
    int get_id(void);

    /**
     * Set router id
     */
    void set_id(int id);

    /**
     * Eject a packet from the router
     */
    mem_req_s* receive_req(int dir = -1);

    /**
     * Pop a request
     */
    void pop_req(int dir = -1);

    /**
     * Initialize a router
     */
    void init(int total_router, int* total_packet, pool_c<flit_c>* flit_pool, pool_c<credit_c>* credit_pool);

    /**
     * Set links for a router
     */
    void set_link(int dir, router_c* link);

    /**
     * Printer the router information
     */
    void print(ofstream& out);

    /**
     * Print link information
     */
    void print_link_info();

    void set_dstream_routers(vector<router_c *>&, int);

    void set_ustream_routers(vector<router_c *>&, int);

  private:
    /**
     * RC (Route Calculation) stage
     */
    void stage_rc(void);

    /**
     * Mesh Routing Calculation
     */
    void rc_mesh(void);

    /**
     * Ring Routing Calculation
     */
    void rc_ring(void);
    
    /**
     * VCA (Virtual Channel Allocation) stage
     */
    void stage_vca(void);

    /**
     * VC arbitration
     */
    void stage_vca_pick_winner(int, int, int&, int&);

    /**
     * SWA (Switch Allocation) stage
     */
    void stage_sa(void);

    /**
     * SW arbitration
     */
    void stage_sa_pick_winner(int, int&, int&, int);

    /**
     * SWT (Switch Traversal) stage
     */
    void stage_st(void);

    /**
     * LT (Link Traversal) stage
     */
    void stage_lt(void);

    /**
     * Get output virtual channel occupancy per type
     */
    int get_ovc_occupancy(int port, bool type);

    /**
     * Check starvation
     */
    void check_starvation(void);

    /**
     * Check channel (for statistics)
     */
    void check_channel(void);

    /**
     * Process pending credits to model credit traversal latency
     */
    void process_pending_credit(void);

    /**
     * Local packet injection
     */
    void local_packet_injection(void);

    void packet_injection(void);

    bool try_packet_insert(int src, mem_req_s *req);

    /**
     * Insert a credit from previous router
     */
    void insert_credit(credit_c*);

  private:
    macsim_c* m_simBase; /**< pointer to simulation base class */
    string m_topology; /**< router topology */
    int m_type; /**< router type */
    int m_id; /**< router id */
    int m_total_router; /**< number of total routers */

    // configurations
    int m_num_vc; /**< number of virtual channels */
    int m_num_port; /**< number of ports */
    int m_link_latency; /**< link latency */
    int m_link_width; /**< link width */
    int* m_total_packet; /**< number of total packets (global) */
    bool m_enable_vc_partition; /**< enable virtual channel partition */
    int m_num_vc_cpu; /**< number of vcs for CPU */
    int m_next_vc; /**<id of next vc for local packet injection */
 
    // pools for data structure
    pool_c<flit_c>* m_flit_pool; /**< flit data structure pool */
    pool_c<credit_c>* m_credit_pool; /**< credit pool */
    
    // arbitration
    int m_arbitration_policy; /**< arbitration policy */

    // link
    router_c* m_link[5]; /**< links */
    unordered_map<int, int> m_opposite_dir; /**< opposite direction map */

    // buffers
    list<mem_req_s*>* m_injection_buffer; /**< injection queue */
    int m_injection_buffer_max_size; /**< max injection queue size */
    queue<mem_req_s*>* m_req_buffer; /**< ejection queue */

    int m_buffer_max_size; /**< input/output buffer max size */

    // per input port
    list<flit_c*>** m_input_buffer; /**< input buffer */
    bool**** m_route; /**< route information */
    int** m_route_fixed; /**< determined rc for the packet */
    int** m_output_port_id; /**< output port id */
    int** m_output_vc_id; /**< output vc id */
    
    // per output port
    list<flit_c*>** m_output_buffer; /**< output buffer */
    bool** m_output_vc_avail; /**< output vc availability */
    int** m_credit; /**< credit counter for the flow control */
    Counter* m_link_avail; /**< link availability */

    // per switch
    Counter* m_sw_avail; /**< switch availability */

    // credit-based flow control
    list<credit_c*>* m_pending_credit; /**< pending credit to model latency */

    // clock
    Counter m_cycle; /**< router clock */

    // additions for validation
    vector<router_c *> m_dstream; /**< down stream routers */
    int m_ds_start;
    vector<router_c *> m_ustream; /**< up stream routers */
    int m_us_start;
    queue<mem_req_s *> m_us_injection_buffer;
    queue<mem_req_s *> m_ds_injection_buffer;
    int m_ds_injection_buffer_max_size;
    int m_us_injection_buffer_max_size;

    Counter m_recv_from_us_avail;
    Counter m_recv_from_ds_avail;
    Counter m_send_to_us_avail;
    Counter m_send_to_ds_avail;

    queue<mem_req_s *> m_packets_from_us;
    queue<mem_req_s *> m_packets_from_ds;
    int m_max_pending_flits_from_us;
    int m_max_pending_flits_from_ds;
    int m_pending_flits_from_us;
    int m_pending_flits_from_ds;
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Router Wrapper Class
///////////////////////////////////////////////////////////////////////////////////////////////
class router_wrapper_c
{
  public:
    /**
     * Constructor
     */
    router_wrapper_c(macsim_c* simBase);

    /**
     * Destructor
     */
    ~router_wrapper_c();

    /**
     * Run a cycle
     */
    void run_a_cycle(void);

    /**
     * Create a router
     */
    router_c* create_router(int type);

    /**
     * Initialize interconnection network
     */
    void init(void);

    /**
     * Ring initialization
     */
    void init_ring(void);

    /**
     * Mesh initialization
     */
    void init_mesh(void);

    /**
     * simple topology for validation
     */
    void init_simple_topology(void);

    /**
     * Print all router information
     */
    void print(void);

    void insert_into_router_req_buffer(int router_id, mem_req_s *req);

  private:
    router_wrapper_c(); // do not implement

  private:
    macsim_c* m_simBase; /**< sim base */
    string m_topology; /**< topology */
    vector<router_c*> m_router; /**< all routers */
    int m_num_router; /**< number of routers */
    int m_total_packet; /**< number of total packets */
    pool_c<flit_c>* m_flit_pool; /**< flit data structure pool */
    pool_c<credit_c>* m_credit_pool; /**< credit pool */

    Counter m_cycle; /**< clock cycle */

    vector<router_c*> m_cores;
    vector<router_c*> m_caches;
    vector<router_c*> m_mcs;
};

#endif
