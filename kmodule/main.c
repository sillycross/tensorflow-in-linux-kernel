#include "indigo_utils.h"

#include "indigo_nn.generated.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm-generic/div64.h>
#include <asm-generic/int-ll64.h>
#include <asm/fpu/api.h>

#define input_length 9
#define output_length 5
#define float_len 8

void print_float(char* buf, float value)
{
	int digits;
	int i;
	int k;
	
	digits = 1;
	while (value >= 10) value /= 10, digits++;
	
	Assert(digits <= float_len - 2);
	
	i = 0;
	for (k = 0; k < float_len - 2; k++)
	{
		buf[i] = '0';
		while (value >= 1 && buf[i] < '9')
		{
			buf[i]++;
			value -= 1;
		}
		i++;
		digits--;
		if (digits == 0)
		{
			buf[i] = '.';
			i++;
		}
		value *= 10;
	}
	Assert(i == float_len - 1);
	buf[i] = 0;
}

static int __init mod_init_fn(void)
{
    struct indigo_nn nn;
    float* data_in;
    float* state_in;
    float* data_out;
    float* state_out;
    int i;
    int iter;
    int lstm_state_size;
    char buf[output_length][float_len];
    
    TRACE_DEBUG("Running neural network in kernel..");
    if (!indigo_nn_constructor(&nn))
    {
        TRACE_INFO("Failed to initialize NN: OOM");
        return -EINVAL;
    }
    
    kernel_fpu_begin();
    
    // Initialize input vector
    //
    data_in = indigo_nn_get_address_for_argument(&nn, 0);
    for (i = 0; i < input_length; i++)
    {
    	data_in[i] = 0.1 * i;
    }
    // Initialize LSTM internal state to 0
    //
    lstm_state_size = indigo_nn_get_argument_size_bytes(1);
    state_in = indigo_nn_get_address_for_argument(&nn, 1);
    memset(state_in, 0, lstm_state_size);
    
    for (iter = 0; iter < 10; iter++)
    {
    	indigo_nn_run(&nn);
    	data_out = indigo_nn_get_address_for_result(&nn, 0);
    	state_out = indigo_nn_get_address_for_result(&nn, 1);
    	
    	for (i = 0; i < output_length; i++)
    	{
    		print_float(buf[i], data_out[i]);
    	}
    	
    	// We hard code output length = 5 here..
    	//
    	TRACE_INFO("Iteration %d: output is %s %s %s %s %s", 
    				iter, buf[0], buf[1], buf[2], buf[3], buf[4]);
    				
    	// Copy state_out to state_in for next iteration
    	//
    	memcpy(state_in, state_out, lstm_state_size);
    }
    kernel_fpu_end();
    
    indigo_nn_destructor(&nn);
    return 0;
}

static void __exit mod_exit_fn(void)
{
    TRACE_DEBUG("Unregistering Kernel Module..");
}

module_init(mod_init_fn);
module_exit(mod_exit_fn);

MODULE_AUTHOR("X");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("X");
 
