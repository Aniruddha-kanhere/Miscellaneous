simin = [0 11010; 1 10101; 2 01010; 3 00110;4 00100];
simout = sim('Kanhere_HW4_Problem4_b','StartTime','0','StopTime','4','FixedStep','1');
dec_simin = [26 21 10 6 4];
stem(dec_simin,simout.simout.Data);
xlabel('Digital input (decimal scale)');
ylabel('Analog output');
grid ON;
grid MINOR

