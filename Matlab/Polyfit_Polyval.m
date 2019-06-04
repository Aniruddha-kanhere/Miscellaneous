voltage_percentage = 10:10:100;
w1 = [1.05, 2.56, 4.07, 5.58, 7.19, 8.66, 10.27, 11.77, 13.26, 15.03];
w2 = [1.13, 2.68, 4.21, 5.76, 7.42, 8.97, 10.57, 12.06, 13.51, 15.42];
sensitivity = zeros(9,1);
w_avg = (w1 + w2)/2;

for i=2:10
   sensitivity(i-1) = (w_avg(i) - w_avg(i-1))/(voltage_percentage(i) - voltage_percentage(i-1));
end

[P] = polyfit(voltage_percentage,w1,1);
linear_value_m1 = polyval(P, voltage_percentage);

[P] = polyfit(voltage_percentage,w2,1);
linear_value_m2 = polyval(P, voltage_percentage);

non_linear_error_m1 = w1 - linear_value_m1;
non_linear_error_m2 = w2 - linear_value_m2;

figure(1);
plot(voltage_percentage(2:10),sensitivity, '-ro');
xlabel("Voltage percentage");
ylabel("Sensitivity(rad/V(%))");
title("(b) Sensitivity");

figure(2);
plot(voltage_percentage, non_linear_error_m1, '-^r', voltage_percentage, non_linear_error_m2, '-b');
xlabel("Voltage percentage");
ylabel("Non linear error(rad/s)");
title("(c) Non linearity error");
legend('Non linearity error m1','Non linearity error m2');

figure(3);
plot(voltage_percentage, w1, '-xr')
hold on;
yyaxis('right');
ylim([-0.2, 0.2]);
ylabel("Non linear error(rad/s)");
plot(voltage_percentage, non_linear_error_m1, '-ob');
xlabel("Voltage percentage");
yyaxis('left');
ylabel("Motor speed(rad/s)", 'color', 'r');
title("(d) Data and error");
legend('Actual value','Non linearity error m1','Location','southeast');