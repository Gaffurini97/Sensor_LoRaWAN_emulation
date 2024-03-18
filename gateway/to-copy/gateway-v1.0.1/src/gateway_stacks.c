#include "../headers/gateway_stacks.h"

int initialize_stacks(){
    rsp_r = 0;
    sp_r = 0;
    stack_r = malloc(sizeof(struct lgw_pkt_rx_s)*PKT_STACK_SIZE);
    if(stack_r == NULL)return -1;
    return 0;
}

int get_remaining_slots_read(int sp, int rsp, int size){
    int remaining_slots = sp - rsp;
    if(remaining_slots == 0){
        //printf("No packets\n");
        return 0;// 0 = no packets
    }
    else if(remaining_slots < 0){
        //If: stack_to_consume < 0
            //means that it's not-ordinary situation
            //Example with cyclic stack dim = 10.
            //| 0 1 2 3 4 5 6 7 8 9 | <--- indexes
            //| n n r r r r r r n n | <--- sp = 1 (index), rcvsp = 7(index too).
            //in the upper case:
            //stack_to_consume = 1-7 = -6
            //(new) stack_to_consume = 10 + (-6) = 4 ('n' count in example stack)
        //End-if.
        remaining_slots = size + remaining_slots;
    }
    //Else:
        //means that it's normal situation (r = received, n = new, - = empty):
        //Example with cyclic stack dim = 10.
        //| 0 1 2 3 4 5 6 7 8 9 | <--- indexes
        //| r r r r r r n n n - | <--- sp = 8 (index), rcvsp = 5.
    //End-else.
    return remaining_slots;
}
int get_remaining_slots_push(int sp, int rsp, int size){
    return size - get_remaining_slots_read(sp,rsp,size);
}

StackStatus push_r(struct lgw_pkt_rx_s pkt_data){
    pthread_mutex_lock(&mtx_r);
    int n = get_remaining_slots_push(sp_r, rsp_r, PKT_STACK_SIZE);
    if(n > 0){
        stack_r[sp_r++] = pkt_data;
        if(sp_r == PKT_STACK_SIZE){
            sp_r = 0;
        }
        pthread_mutex_unlock(&mtx_r);
        return STACK_OK;
    }else{
        pthread_mutex_unlock(&mtx_r);
        return STACK_FULL;
    }
}
StackStatus pop_r(struct lgw_pkt_rx_s *pkt_data){
    pthread_mutex_lock(&mtx_r);
    int n = get_remaining_slots_read(sp_r, rsp_r, PKT_STACK_SIZE);
    if(n > 0){
        memcpy(pkt_data, &stack_r[rsp_r], sizeof(stack_r[rsp_r]));
        rsp_r++;
        if(rsp_r == PKT_STACK_SIZE){
            rsp_r = 0;
        }
        pthread_mutex_unlock(&mtx_r);
        return STACK_OK;
    }else{
        pthread_mutex_unlock(&mtx_r);
        return STACK_EMPTY;
    }
}