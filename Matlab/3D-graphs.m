%% Initialize Parameters
clc;
clear;
I=0.1:0.01:20; %set the voltage range
IS=25e-9; %set the reverse current
N = 3; %set the emission coefficient
Vt = [21:0.1:25]; %set the thermal voltage range, mV
%% Plot I to V-Vt ECE456/556 Mechatronics, HW 2 3
[X1,Y1]=meshgrid(I,Vt); %produce the coordinates of a rectangular grid from range of voltage and thermal voltage
V= (N*10^(-3)*Y1) .* log((X1/IS) + 1);
figure(1); %create a new figure;
mesh(X1,Y1,V); %draw the 3D graph
zlabel('Voltage (v)'); %label the x-axis
ylabel('Thermal voltage (mV)'); %label the y-axis
xlabel('Current (A)'); %label the z-axis