%%
clear all;
clc;

f_s=50;
t1 = -1:1/f_s:1-1/f_s;
x1 = cos(10*pi*t1);
figure;
stem(t1,x1);
figure;
plot(t1,x1);
y1=fftshift(fft(x1));
%N=size(y1,2);
N=2*f_s;
f1= -f_s/2:f_s/N:f_s/2-f_s/N;
y1 = y1/max(abs(y1));
%%
figure;
plot(f1,abs(y1));
% xlabel('Frequency (Hz)');
% ylabel('|X(f)|');
% title('Magnitude');
% grid on;

% subplot(1,2,2);
% plot(f1,angle(y1));
% xlabel('Frequency (Hz)');
% ylabel('Phase of X(f)');
% title('Phase');
% grid on;

%% rggh

f_s_2 = 100;

t2 = 0:1/f_s_2:1-1/f_s_2;
x2= cos(30*pi*t2+pi/4);
figure;
stem(t2,x2);
figure;
plot(t2,x2);
%%
y2=fftshift(fft(x2));
% N2=size(y2,2);
N2= f_s_2*(1);
f2= -f_s_2/2:f_s_2/N2:(f_s_2/2-f_s_2/N2);
y2 = y2/max(abs(y2));
figure;
plot(f2,abs(y2));
%%
% xlabel('Frequency (Hz)');
% ylabel('|X(f)|');
% title('Magnitude');

% subplot(1,2,2);
% plot(f2,angle(y2));
% xlabel('Frequency (Hz)');
% ylabel('Phase of X(f)');
% title('Phase');
% grid on;

tol = 1e-6;
y2(abs(y2) < tol) = 0;
theta = angle(y2);
figure;
plot(f2,theta/pi)
xlabel 'Frequency (Hz)'
ylabel 'Phase / \pi'
