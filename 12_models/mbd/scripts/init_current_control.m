%% FOC Current Controller Parameters

control = struct();

%% Fast control loop

control.currentLoopFrequency_Hz = 20.0e3;
control.currentLoopPeriod_s = 1.0 / control.currentLoopFrequency_Hz;

%% Current-Loop bandwidth target

control.currentBandwidth_Hz = 1.0e3;
control.currentBandwidth_radps = control.currentBandwidth_Hz * 2.0 * pi;

%% Nominal d-axis PI gains after zero pole cancellation

control.idKp = motor.Ld_H * control.currentBandwidth_radps;
control.idKi = motor.Rs_Ohm * control.currentBandwidth_radps;

%% Nominal q-axis PI gains after zero pole cancellation 

control.iqKp = motor.Lq_H * control.currentBandwidth_radps;
control.iqKi = motor.Rs_Ohm * control.currentBandwidth_radps;
