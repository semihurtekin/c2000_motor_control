%% LVSERVOMTR PMSM Plant MIL Validation

run("../scripts/init_lvservomtr.m");

modelName = "tb_lvservomtr_plant";

simOut = sim( ...
    modelName, ...
    "StopTime", "0.08", ...
    "ReturnWorkspaceOutputs", "on");

id     = simOut.idLog;
iq     = simOut.iqLog;
te     = simOut.teLog;
speedM = simOut.speedLog;

%% Evaluate final steady-state region

steadyStateStart_s = 0.075;

idIdx = id.Time >= steadyStateStart_s;
iqIdx = iq.Time >= steadyStateStart_s;
teIdx = te.Time >= steadyStateStart_s;
speedIdx = speedM.Time >= steadyStateStart_s;

idSs_A = mean(id.Data(idIdx));
iqSs_A = mean(iq.Data(iqIdx));
teSs_Nm = mean(te.Data(teIdx));

speedSs_radps = mean(speedM.Data(speedIdx));

speedSs_rpm = ...
    speedSs_radps * 60.0 / (2.0 * pi);

%% Expected analytical operating point

expectedIq_A = 1.302;
expectedId_A = 0.284;
expectedTe_Nm = 0.050;
expectedSpeed_rpm = 936.0;

%% Tolerances

iqTolerance_A = 0.05;
idTolerance_A = 0.05;
teTolerance_Nm = 0.002;
speedTolerance_rpm = 10.0;

%% Assertions

assert( ...
    abs(iqSs_A - expectedIq_A) < iqTolerance_A, ...
    "Iq steady-state validation failed.");

assert( ...
    abs(idSs_A - expectedId_A) < idTolerance_A, ...
    "Id steady-state validation failed.");

assert( ...
    abs(teSs_Nm - expectedTe_Nm) < teTolerance_Nm, ...
    "Torque steady-state validation failed.");

assert( ...
    abs(speedSs_rpm - expectedSpeed_rpm) < speedTolerance_rpm, ...
    "Speed steady-state validation failed.");

disp("LVSERVOMTR plant MIL validation: PASS");