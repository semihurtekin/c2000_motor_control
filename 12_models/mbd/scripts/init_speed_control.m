%% Speed-loop parameters
% Run init_lvservomtr.m and init_current_control.m first, or use init_mbd.m.

assert(exist('motor', 'var') == 1 && isstruct(motor), ...
    'Run init_lvservomtr.m before init_speed_control.m.');
assert(exist('control', 'var') == 1 && isstruct(control), ...
    'Run init_current_control.m before init_speed_control.m.');

speedControl = struct();

%% Design settings

% This is the P-only bandwidth target, not the measured PI bandwidth.
speedControl.proportionalBandwidth_Hz = 100.0;
speedControl.integralZeroRatio = 0.1;

% The 2 A limit belongs to the present simulation experiment.
speedControl.iqLimit_A = 2.0;
speedControl.antiWindupEnable = 1;

% Keep both loops at the same rate for now. Multirate execution comes later.
speedControl.sampleTime_s = control.currentLoopPeriod_s;

%% Motor and frequency terms

% Te = Kt * iq for the present idRef = 0 operating condition.
speedControl.torqueConstant_NmPerA = ...
    1.5 * motor.polePairs * motor.psiPm_Wb;

assert(motor.J_kgm2 > 0 && speedControl.torqueConstant_NmPerA > 0, ...
    'Motor inertia and torque constant must be positive.');

speedControl.proportionalBandwidth_radps = ...
    2.0 * pi * speedControl.proportionalBandwidth_Hz;

speedControl.piZero_Hz = speedControl.integralZeroRatio * ...
    speedControl.proportionalBandwidth_Hz;
speedControl.piZero_radps = 2.0 * pi * speedControl.piZero_Hz;

%% PI gains

% Initial tuning assumes ideal inner current tracking and negligible friction.
% Keep the previous P tuning and place the PI zero below that frequency.
speedControl.Kp = motor.J_kgm2 * ...
    speedControl.proportionalBandwidth_radps / ...
    speedControl.torqueConstant_NmPerA;
speedControl.Ki = speedControl.Kp * speedControl.piZero_radps;

%% Back-calculation

% Integrate Ki * speedError + Kaw * (iqLimited - iqRaw).
% The integral state is a current contribution in amperes.
speedControl.Kaw = speedControl.Ki / speedControl.Kp;
