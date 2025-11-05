clear
clc
close all



%% Set up arduino

port = '/dev/cu.usbmodem1301'; % which port you're connected to
pause(2)
%board = 'Uno'; % the kind of board you're using?

%a = arduino(port, board);

uno = serialport(port, 115200);
configureTerminator(uno, 62); % Wait for the end character '>'
uno.Timeout = 15; % Add a timeout of 5 seconds

%% Set up figures

figure();
grid on;

clf
t = tiledlayout(4,2);
title(t,"Test stand data")
tiledlayout("flow")
grid on;

nexttile(1);
nexttile(2);
nexttile(3);
nexttile(4);
nexttile(5);
nexttile(6);
nexttile(7);


s1 = 0;
s2 = 0;
s3 = 0;
s4 = 0;
s5 = 0;
s6 = 0;
s7 = 0;
s8 = 0;
s9 = 0;
s10 = 0;


data = [0 0 0 0 0 0 0 0];
i = 1;

%% Run plot


while true
    %% Read data
        % Read the entire data packet until the '>' terminator
        dataString = readline(uno); 
        flush(uno);
     
        

        if startsWith(dataString, '<')

            % Remove the start and end characters
            dataString = erase(dataString, '<');
            dataString = erase(dataString, '>');
            
            % Split the string by the comma delimiter
            pressureStrings = split(dataString, ',');
            
            % Convert the string array to a numerical matrix (1x7)
            dataMatrix = str2double(pressureStrings)';
    
            if length(dataMatrix) == 10
                %% Sort data
            
                Pt_volt1 = dataMatrix(1); 
                Pt_volt2 = dataMatrix(2); 
                Pt_volt3 = dataMatrix(3); 
                Pt_volt4 = dataMatrix(4); 
                Pt_volt5 = dataMatrix(5); 
                Pt_volt6 = dataMatrix(6); 
                Pt_volt7 = dataMatrix(7); 

                Load1 = dataMatrix(8);
                Load2 = dataMatrix(9);
                Load3 = dataMatrix(10);
            
            
                %% Calibrate pressure
            
                pressure1 = Pt_volt1 * 0.069307 - 139.1386; % Need to determine conversion and offest experimentally
                pressure2 = Pt_volt2 * 0.069307 - 139.1386; % Factors will likely be different for each sensor
                pressure3 = Pt_volt3 * 0.069307 - 139.1386;
                pressure4 = Pt_volt4 * 0.069307 - 139.1386 - 1;
                pressure5 = Pt_volt5 * 0.069307 - 139.1386;
                pressure6 = Pt_volt6 * 0.069307 - 139.1386;
                pressure7 = Pt_volt7 * 0.069307 - 139.1386;
            
            
            
                %% Update data
            
                nexttile(1)
                s1 =[s1,pressure1];
                plot(s1);
                title("PT 1")
                xlabel('Time')
                ylabel("Engine: Fuel Inlet")
                grid on;
            
                
                nexttile(2)
                s2 =[s2,pressure2];
                plot(s2);
                title("PT 2")
                xlabel('Time')
                ylabel("Engine: Fuel Injector")
                grid on;

                
                nexttile(3)
                s3 =[s3,pressure3];
                plot(s3);
                title("PT 3")
                xlabel('Time')
                ylabel("Engine: LOX Inlet")
                grid on;
                
            
                nexttile(4)
                s4 =[s4,pressure4];
                plot(s4);
                title("PT 4")
                xlabel('Time')
                ylabel("Tank: Fuel")
                grid on;
            
                nexttile(5)
                s5 =[s5,pressure5];
                plot(s5);
                title("PT 5")
                xlabel('Time')
                ylabel("Tank: LOX")
                grid on;
                
                %{
                nexttile(6)
                s6 =[s6,pressure6];
                plot(s6);
                title("PT 1")
                xlabel('Time')
                ylabel("Pressure(psi)")
                grid on;
            
                nexttile(7)
                s7 =[s7,pressure7];
                plot(s7);
                title("PT 1")
                xlabel('Time')
                ylabel("Pressure(psi)")
                grid on;
                %}
                
                nexttile(6)
                s8 =[s8,Load1];
                plot(s8);
                title("Load Cell 1")
                xlabel('Time')
                ylabel("Load (lbf)")
                grid on;
                
                nexttile(7)
                s9 =[s9,Load2];
                plot(s9);
                title("Load Cell 2")
                xlabel('Time')
                ylabel("Load (lbf)")
                grid on;
                
                nexttile(8)
                s10 =[s10,Load3];
                plot(s10);
                title("Load Cell 3")
                xlabel('Time')
                ylabel("Load (lbf)")
                grid on;
            
                drawnow;
                
                data(i, 1) = pressure1;
                data(i, 2) = pressure2;
                data(i, 3) = pressure3;
                data(i, 4) = pressure4;
                data(i, 5) = pressure5;
                data(i, 6) = Load1;
                data(i, 7) = Load2;
                data(i, 8) = Load3;

                i = i + 1;

            end

        end
 end
