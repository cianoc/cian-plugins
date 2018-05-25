#include "SC_PlugIn.hpp"
#include "SC_SndBuf.h"

// InterfaceTable contains pointers to functions in the host (server).
static InterfaceTable *ft;

// declare struct to hold unit generator state
struct SinOp : public SCUnit{

// Constructor usually does 3 things.
// 1. set the calculation function.
// 2. initialize the unit generator state variables.
// 3. calculate one sample of output.
public:
    SinOp() 
        : radtoinc(ft->mSineSize * (rtwopi * 65536.)) 
        , cpstoinc (ft->mSineSize * sampleDur() * 65536) 
        ,  lomask ((ft->mSineSize - 1) << 3)
    {
        // get initial phase of oscillator

        // 1. set the calculation function.
        if (isAudioRateIn(0)) { // if the frequency argument is audio rate
            if(isAudioRateIn(1)) // if the phase argument is audio rate
                set_calc_function<SinOp,&SinOp::next_aa>();
            else
                set_calc_function<SinOp,&SinOp::next_ak>();                
        } else {    
            if(isAudioRateIn(1)) // if the phase argument is audio rate
                set_calc_function<SinOp,&SinOp::next_ka>();
            else{
                set_calc_function<SinOp,&SinOp::next_kk>();                
            }
        }

        phase = 0;
        prevout = 0.;

        p_freq = in0(0);
        p_phasein = in0(1);

        // calculate one sample of output.
        next_kk(1);
    }

private:
    int32 phase;

    float prevout;

    float p_freq;
    float p_phasein;

    const int32 lomask;
    const double radtoinc;
    const double cpstoinc;

    //////////////////////////////////////////////////////////////////

    // The calculation function executes once per control period
    // which is typically 64 samples.

    // calculation function for an audio rate frequency argument
    void next_aa(int inNumSamples)
    {
        // get access to the built in sinewave table.
        float *table0 = ft->mSineWavetable;
        float *table1 = table0 + 1;

        float *buf_out = out(0);

        const float* freq_in = in(0);
        const float *phase_in = in(1);
        float feedback = in0(2);

        int32 l_phase = phase;
        float l_prevout = prevout;

        float fb;
        for (int i=0; i < inNumSamples; ++i)
        {
            if (feedback > 0)
                fb = l_prevout * feedback;
            else if (feedback < 0)
                fb = l_prevout * l_prevout * feedback;
            else 
                fb = 0.0;
  
            l_phase += (int32)(freq_in[i]*cpstoinc);           

            int32 phaseoffset = (int32) ((phase_in[i] + fb)*radtoinc);

            l_prevout = lookupi1(table0, table1, l_phase + phaseoffset, lomask);

            buf_out[i] = l_prevout;
        }

        prevout = l_prevout;
        phase = l_phase;

    }

    void next_ak(int inNumSamples)
    {
        // get access to the built in sinewave table.
        float *table0 = ft->mSineWavetable;
        float *table1 = table0 + 1;

        float *buf_out = out(0);

        const float *freq_in = in(0);
        float phase_in = in0(1);
        float feedback = in0(2);

        int32 l_phase = phase;
        float l_prevout = prevout;

        float l_phase_in = p_phasein * radtoinc;

        float phase_slope=0.0;

        if (phase_in != p_phasein)
            phase_slope = calcSlope(phase_in, p_phasein) * radtoinc;

        float fb;
        for (int i=0; i < inNumSamples; ++i)
        {
            if (feedback > 0)
                fb = l_prevout * feedback;
            else if (feedback < 0)
                fb = l_prevout * l_prevout * feedback;
            else 
                fb = 0.0;
  
            l_phase += (int32)(freq_in[i]*cpstoinc);           

            int32 phaseoffset = (int32) (l_phase_in + (fb*radtoinc));
            l_phase_in += phase_slope;

            l_prevout = lookupi1(table0, table1, l_phase + phaseoffset, lomask);

            buf_out[i] = l_prevout;
        }

        prevout = l_prevout;
        phase = l_phase;

        p_phasein = phase_in;

    }
        

    void next_ka(int inNumSamples)
    {
        // get access to the built in sinewave table.
        float *table0 = ft->mSineWavetable;
        float *table1 = table0 + 1;

        float *buf_out = out(0);

        float freq_in = in0(0);
        const float *phase_in = in(1);
        float feedback = in0(2);

        int32 l_phase = phase;
        float l_prevout = prevout;

        float l_freq = p_freq * cpstoinc;

        float feedback_slope=0.0;
        float freq_slope=0.0;

        if (freq_in != l_freq)
            freq_slope = calcSlope(freq_in, p_freq) * cpstoinc;

        float fb;
        for (int i=0; i < inNumSamples; ++i)
        {
            if (feedback > 0)
                fb = l_prevout * feedback;
            else if (feedback < 0)
                fb = l_prevout * l_prevout * feedback;
            else 
                fb = 0.0;

 
            l_phase += (int32)(l_freq);           
            l_freq += freq_slope;

            int32 phaseoffset = (int32) ((phase_in[i] + fb)*radtoinc);

            l_prevout = lookupi1(table0, table1, l_phase + phaseoffset, lomask);

            buf_out[i] = l_prevout;
        }

        prevout = l_prevout;
        phase = l_phase;

        p_freq = freq_in;


    }

    void next_kk(int inNumSamples)
    {
        // get access to the built in sinewave table.
        float *table0 = ft->mSineWavetable;
        float *table1 = table0 + 1;

        float *buf_out = out(0);

        float freq_in = in0(0);
        float phase_in = in0(1);
        float feedback = in0(2);

        int32 l_phase = phase;
        float l_prevout = prevout;

        float l_phase_in = p_phasein * radtoinc;
        float l_freq = p_freq * cpstoinc;

        float phase_slope=0.0;
        float freq_slope=0.0;

        if (freq_in != l_freq)
            freq_slope = calcSlope(freq_in, p_freq) * cpstoinc;

        if (phase_in != p_phasein)
            phase_slope = calcSlope(phase_in, p_phasein) * radtoinc;

        float fb;
        for (int i=0; i < inNumSamples; ++i)
        {
            if (feedback > 0)
                fb = l_prevout * feedback;
            else if (feedback < 0)
                fb = l_prevout * l_prevout * feedback;
            else 
                fb = 0.0;

 
            l_phase += (int32)(l_freq);           
            l_freq += freq_slope;

            int32 phaseoffset = (int32) (l_phase_in + (fb*radtoinc));
            l_phase_in += phase_slope;

            l_prevout = lookupi1(table0, table1, l_phase + phaseoffset, lomask);

            buf_out[i] = l_prevout;
        }

        prevout = l_prevout;
        phase = l_phase;

        p_freq = freq_in;
        p_phasein = phase_in;

    }

};

// the entry point is called by the host when the plug-in is loaded
PluginLoad(CianOpUGens)
{
    // InterfaceTable *inTable implicitly given as argument to the load function
    ft = inTable; // store pointer to InterfaceTable

    // registerUnit takes the place of the Define*Unit functions. It automatically checks for the presence of a
    // destructor function.
    // However, it does not seem to be possible to disable buffer aliasing with the C++ header.
    registerUnit<SinOp>(ft, "SinOp");
}
