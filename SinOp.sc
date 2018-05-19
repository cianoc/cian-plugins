// without mul and add.
SinOp : UGen {
    *ar { arg freq = 440.0, phase = 0.0, feedback = 0.0;
        ^this.multiNew('audio', freq, phase,feedback)
    }
}