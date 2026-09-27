%% LVSERVOMTR - Teknic M-2310P-LN-04K
% Motor parameters used by the equation-based PMSM plant.
%
% Source values:
%   Phase-to-phase resistance  : 0.72 ohm
%   Phase-to-phase inductance  : 0.40 mH
%   Back-EMF constant          : 4.64 Vpeak/kRPM
%   Number of pole pairs       : 4
%
% Phase quantities used by the dq model are derived below.

motor = struct();

%% Electrical parameters

motor.polePairs = 4;

motor.RphasePhase_Ohm = 0.72;
motor.LphasePhase_H   = 0.40e-3;

motor.Rs_Ohm = motor.RphasePhase_Ohm / 2.0;

motor.Ld_H = motor.LphasePhase_H / 2.0;
motor.Lq_H = motor.LphasePhase_H / 2.0;

motor.psiPm_Wb = 6.40e-3;

%% Derived electrical quantities

motor.electricalTimeConstant_s = ...
    motor.Ld_H / motor.Rs_Ohm;

%% Initial mechanical model parameters

motor.J_kgm2 = 7.0616e-6;
motor.B_Nms  = 0.0;

%% Initial conditions

motor.id0_A       = 0.0;
motor.iq0_A       = 0.0;
motor.speed0_radps = 0.0;
motor.theta0_rad   = 0.0;
