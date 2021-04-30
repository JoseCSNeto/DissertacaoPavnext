clear;
filename = 'v2i2e2corrected';

fileID = fopen(strcat('reads\',filename,'.txt'),'r');
file_content = textscan( fileID, '%s', 'Delimiter', '\n' );
fclose(fileID);
[lines,column] = size(file_content{1,1});
i = 0;
readInterval = 1.65;
factor = 0.30625;

string_limit = length(file_content{1, 1}{5, 1});
counter = extractBetween(file_content{1, 1}{5, 1},8,string_limit); %count
counter=str2double(counter);

string_limit = length(file_content{1, 1}{7, 1});
stopReadingAccel = extractBetween(file_content{1, 1}{7, 1},7,string_limit); %stop
stopReadingAccel=str2double(stopReadingAccel);

string_limit = length(file_content{1, 1}{11, 1});
firstReadCounter = extractBetween(file_content{1, 1}{11, 1},17,string_limit); %firstReadCount
firstReadCounter_num = str2double(firstReadCounter);

secondReadCounter_num = counter - firstReadCounter_num;

string_limit = length(file_content{1, 1}{13, 1});
firstReadTimer = extractBetween(file_content{1, 1}{13, 1},20,string_limit); %firstReadDuration
firstReadTimer_num = str2double(firstReadTimer)*10^-3;
firstReadTimer_num=ceil(firstReadTimer_num);

firstReadInterval = firstReadTimer_num/firstReadCounter_num;

string_limit = length(file_content{1, 1}{17, 1});
secondReadTimer = extractBetween(file_content{1, 1}{17, 1},21,string_limit); %secondReadDuration
secondReadTimer_num = str2double(secondReadTimer)*10^-3;
secondReadTimer_num=ceil(secondReadTimer_num);

secondReadInterval = secondReadTimer_num/secondReadCounter_num;

totalTimerSeconds = (firstReadTimer_num + secondReadTimer_num)*10^-3;

string_limit = length(file_content{1, 1}{19, 1});
intervalBetweenDetections = extractBetween(file_content{1, 1}{19, 1},28,string_limit); %intervalBetweenDetections
intervalBetweenDetections_num = str2double(intervalBetweenDetections)*10^-3;
intervalBetweenDetections_num=ceil(intervalBetweenDetections_num);

intervalBetweenDetectionsSeconds = intervalBetweenDetections_num*10^-3;

data_init = 'voltage_V2: ';

voltage_V2 = cell(counter,1);
voltage_V1 = zeros(counter,1);
current_I2 = cell(counter,1);
current_I1 = zeros(counter,1);

voltage_V2_num = zeros(counter,1);
voltage_V1_num = zeros(counter,1);
current_I2_num = zeros(counter,1);
current_I1_num = zeros(counter,1);

power_P1 = zeros(counter,1);
power_P2 = zeros(counter,1);
energy_E1 = zeros(counter,1);
energy_E2 = zeros(counter,1);

zMeasures = cell(counter,1);
zMeasures_num = zeros(counter,1);


for N = 1:lines
    content = file_content{1, 1}{N, 1};
    
    index = strfind(content,data_init);
    if(index)
        i = i + 1;
        start_point = index+length(data_init);
        string_limit = length(file_content{1, 1}{N, 1});
        
        voltage_V2(i) = extractBetween(file_content{1, 1}{N, 1},13,string_limit);
        %         voltage_V1(i) = 0;%extractBetween(file_content{1, 1}{N+1, 1},13,length(file_content{1, 1}{N+1, 1}));
        current_I2(i) = extractBetween(file_content{1, 1}{N+1, 1},13,length(file_content{1, 1}{N+1, 1}));
        %         current_I1(i) = 0;%extractBetween(file_content{1, 1}{N+3, 1},13,length(file_content{1, 1}{N+3, 1}));
        zMeasures(i) = extractBetween(file_content{1, 1}{N+2, 1},13,length(file_content{1, 1}{N+2, 1}));
        
        voltage_V2_num (i) = str2double(voltage_V2(i))*50/1024;
        %         voltage_V1_num (i) = str2double(voltage_V1(i))*50/1024;
        
        current_I2_num (i) = (str2double(current_I2(i))*5/1024)*6.1597 - 15.495;
        
        if (current_I2_num (i) < 0)
            current_I2_num (i) = 0;
        end
        %         current_I1_num (i) = (str2double(current_I1(i))*5/1024)*6.1597 - 15.495;
        zMeasures_num (i) = str2double(zMeasures(i))*factor;
        
        %         power_P1(i) = abs(voltage_V1_num (i) * current_I1_num (i));
        power_P2(i) = abs(voltage_V2_num (i) * current_I2_num (i));
        
    end
end

layout = tiledlayout(5,1);
layout_title = 'Energy Generation';
title(layout,layout_title,'FontWeigh','bold');

x = 1:counter;

formatSpec = '%.2f';
%power1_text = strcat('Power: ', {' '}, num2str (sum(power_P1),formatSpec), {' '}, 'W');
%power_P2_text = strcat('Power: ', {' '}, num2str (sum(power_P2),formatSpec), {' '}, 'W');

%energy_E1 = sum(power_P1)*(timer_num*1*10^-3);
energy_E2 = sum(power_P2)*(totalTimerSeconds);
%energy_E1_total = sum(abs(power_P1(stopReadingAccel+1:counter)*1*10^-3)) + sum(abs(power_P1(1:stopReadingAccel)*readInterval*10^-3));
energy_E2_total = sum(abs(power_P2(1:firstReadCounter_num)*firstReadInterval*10^-3)) +  sum(abs(power_P2(firstReadCounter_num+1:counter)*secondReadInterval*10^-3));
%
%
% energy1_text = strcat('Energy:', {' '}, num2str (energy_E1_total,formatSpec), {' '}, 'J');
energy2_text = strcat('Energy:', {' '}, num2str (energy_E2_total,formatSpec), {' '}, 'J');

% %gerador1
% X11 = zeros(1,stopReadingAccel);
% for c = 1:stopReadingAccel
%     X11(c) = X11(c)+(readInterval*10^-3)*c;
% end
%
% X12 = (X11(end)+1*10^-3):(1*10^-3):timer_num*(1*10^-3);
% X1 = cat(2,X11,X12);
% X1=X1(1:counter);

%gerador2
X21 = zeros(1,firstReadCounter_num);
X22 = zeros(1,secondReadCounter_num);

for c = 1:firstReadCounter_num
    X21(c) = (firstReadInterval*10^-3)*c;
end

for c = 1:secondReadCounter_num
    X22(c) = X21(end) + (secondReadInterval*10^-3)*(c);
end

X2 = cat(2,X21,X22);
X2=X2(1:counter);


% nexttile
% plot(x,voltage_V1_num,'-o');
% ylim([min(voltage_V1_num)-5 max(voltage_V1_num)+5]);
% xlim([1 counter]);
% title('V1 Measurements')
% ylabel('V1','FontWeigh','bold');
% xlabel('Measures','FontWeigh','bold');
% %text(counter-10,max(voltage_V1_num)+5-1,power1_text,'FontWeigh','bold');


nexttile
plot(X2,voltage_V2_num,'.-');
ylim([min(voltage_V2_num)-5 max(voltage_V2_num)+5]);
xlim([X2(1) X2(end)]);
title('V2 Measurements')
ylabel('V2','FontWeigh','bold')
xlabel('Measures','FontWeigh','bold');
yline(0);
%text(counter-10,max(voltage_V2_num)+5-2.5,energy2_text,'FontWeigh','bold');

% nexttile
% plot(x,current_I1_num,'-o');
% ylim([min(current_I1_num)-5 max(current_I1_num)+5]);
% xlim([1 counter]);
% title('I1 Measurements')
% ylabel('I1','FontWeigh','bold')
% xlabel('Measures','FontWeigh','bold');

nexttile
plot(X2,current_I2_num,'.-');
ylim([min(current_I2_num)-5 max(current_I2_num)+5]);
xlim([X2(1) X2(end)]);
title('I2 Measurements')
ylabel('I2','FontWeigh','bold')
xlabel('Measures','FontWeigh','bold');





% nexttile
% plot(X1,power_P1,'-o');
% %ylim([min(power_P1)-5 max(power_P1)+5]);
% xlim([X1(1) X1(counter)]);
% title('P1 Measurements')
% ylabel('P1','FontWeigh','bold')
% xlabel('Time (s)','FontWeigh','bold');

nexttile
plot(X2,power_P2,'.-');
%ylim([min(power_P2)-5 max(power_P2)+5]);
xlim([X2(1) X2(counter)]);
title('P2 Measurements')
ylabel('P2','FontWeigh','bold')
xlabel('Time (s)','FontWeigh','bold');

for c = 1:firstReadCounter_num
    %     energy_E1 (c) = abs(power_P1 (c)*readInterval*10^-3);
    energy_E2 (c) = abs(power_P2 (c)*firstReadInterval*10^-3);
end
for c = firstReadCounter_num+1:counter
    %     energy_E1 (c) = abs(power_P1 (c)*1*10^-3);
    energy_E2 (c) = abs(power_P2 (c)*secondReadInterval*10^-3);
end

% Q = cumtrapz(X1,power_P1);
% nexttile
% plot(X1,Q,'-o');
% %ylim([min(Q)-5 max(Q)+5]);
% xlim([X1(1) X1(counter)]);
% title('E1 Measurements')
% ylabel('E1','FontWeigh','bold')
% xlabel('Time (s)','FontWeigh','bold');
% text(counter-10,max(voltage_V1_num)+5-2.5,energy1_text,'FontWeigh','bold');
% legend(energy1_text,'Location','southeast');

%gerador2
timer21 = zeros(1,firstReadCounter_num);
timer22 = zeros(1,secondReadCounter_num);

for c = 1:firstReadCounter_num
    timer21(c) = (firstReadInterval*10^-3)*c;
end

for c = 1:secondReadCounter_num
    timer22(c) = timer21(end)+intervalBetweenDetectionsSeconds+(secondReadInterval*10^-3)*(c);
end

timer2 = cat(2,timer21,timer22);
timer2=timer2(1:counter);


Q = cumtrapz(X2,power_P2);
nexttile
plot(X2,Q,'.-');
%BreakXAxis(X2,Q,X21(end),X22(1),1);
%ylim([min(Q)-5 max(Q)+5]);
xlim([X2(1) X2(counter)]);
%xticks([timer2(1) timer21(end) timer22(1) timer2(end)])
title('E2 Measurements')
ylabel('E2','FontWeigh','bold')
xlabel('Time (s)','FontWeigh','bold');
%text(counter-10,max(voltage_V1_num)+5-2.5,energy2_text,'FontWeigh','bold');
legendText = strcat('Energy:', {' '},num2str(Q(end)), {' '}, 'J');
legend(legendText,'Location','southeast');

nexttile
plot(X2,zMeasures_num, '.-');
ylim([min(zMeasures_num)-5 max(zMeasures_num) + 5]);
xlim([X2(1) X2(end)]);
title('Z Axis Measurements')
ylabel('Z axis acceleration (m/s)','FontWeigh','bold')
xlabel('Measures','FontWeigh','bold');
yline(9.8);


% nexttile
% f = fit(X',power_P2,'smoothingspline');
% plot(f,X',power_P2)
% int = integrate(f,X,0)
% sum(int)
% sum(power_P2)
% energy2=zeros(1,50);
%
% nexttile
% f = fit(X',power_P2,'smoothingspline');
% plot(f,X',power_P2)
% int = integrate(f,X,0)
% sum(int)
% sum(power_P2)
% nexttile
% plot(X,cumtrapz(X,energy2),'o-')

h=gcf;
%set(h,'Position',[50 50 1200 800]);
set(h,'PaperOrientation','landscape');
print(gcf, '-fillpage', strcat(filename,'.pdf'));
finalFile = strcat('D:\Documentos\Gonçalo\Pavnext\Power\graphs\',strcat(filename,'.pdf'));
movefile(strcat(filename,'.pdf'),finalFile);
winopen(finalFile);

