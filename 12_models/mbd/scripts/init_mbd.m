%% Initialize the PMSM models and controllers
% All four initialization scripts belong in the same scripts folder.
% This reloads the baseline parameters, including zero initial conditions.

mbdScriptsFolder = fileparts(mfilename('fullpath'));

run(fullfile(mbdScriptsFolder, 'init_lvservomtr.m'));
run(fullfile(mbdScriptsFolder, 'init_current_control.m'));
run(fullfile(mbdScriptsFolder, 'init_speed_control.m'));

clear mbdScriptsFolder;
