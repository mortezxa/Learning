clear all;
clc;

message = 'signal';
code = coding_freq(message,1);
figure;
t = 0:1/100:(length(code)-1)/100;
plot(t, code);
xlabel('Time (s)');
ylabel('Amplitude');
title('Encoded Signal');
grid on;
decoded = decoding_freq(code,1);

figure;
code2 = coding_freq(message,5);
t = 0:1/100:(length(code2)-1)/100;
plot(t, code2);
xlabel('Time (s)');
ylabel('Amplitude');
title('Encoded Signal');
grid on;

decodedd = decoding_freq(code2,5);

noised_sgn1 = code + 0.01*(randn(1,length(code)));

noised_sgn2 = code2 + 0.01*(randn(1,length(code2)));

dcd_sgn1 = decoding_freq(noised_sgn1,1);

dcd_sgn2 = decoding_freq(noised_sgn2,5);
%%

%%
flag1 = 0;
flag2 = 0;
for i =  20:2:150

    temp1noise = code + 0.01*i*(randn(1,length(code)));
    temp2noise = code2 + 0.01*i*(randn(1,length(code2)));

    dc1 = decoding_freq(temp1noise , 1);
    dc2 = decoding_freq(temp2noise , 5);


    if dc2 ~= "signal" && flag2==0
        fprintf('decoded message for 1 bit/sec with i= %d is: %s\n: ' , i,dc1);
        fprintf('decoded message for 5 bit/sec with i= %d is: %s\n' , i,dc2);
        flag2=1;
    end
    if dc1 ~= "signal" && flag1==0
        fprintf('decoded message for 1 bit/sec with i= %d is: %s\n: ' , i,dc1);
        fprintf('decoded message for 5 bit/sec with i= %d is: %s\n' , i,dc2);
        flag1=1;
    end
end
%%
function coded = coding_freq(msg,speed)
    load("Mapset.mat");
    speed_freqs = [];
    speed_step = (50/(2^speed+1));
    for i = 1:2^speed
        speed_freqs = [speed_freqs round(i*speed_step)];
    end
    
    len = length(msg);
    binmsg = [];
    for i = 1:len
        disp(msg(i));
        idx = find(strcmp(data(1,:),msg(i)));
        kdx = data{2,idx};
        binmsg = [binmsg data{2,idx}];
%      idx = find(strcmp(map(1,:), msg(i)));
%      binmsg = [binmsg map{2,idx}];   
    end
    t = 0:1/100:len/speed;
    ts = 0:1/100:0.99;
    y = zeros(1, length(t));
    disp(binmsg);
    lenb = length(binmsg);

    for i=1:lenb/speed
        s = binmsg(speed*(i-1) + 1 : speed*i);
        jdx = bin2dec(s)+1;
        ff = speed_freqs(jdx);
        y((i-1)*100+1:i*100)=sin(2*pi*ff*ts);
    end
    coded = y;

end

function decoded= decoding_freq(signal , speed)
    
    F_s = -49:1:50;
    speed_freqs = [];
    speed_step = (50/(2^speed+1));
    for i = 1:2^speed
        speed_freqs = [speed_freqs round(i*speed_step)];
    end
    Fourier = -speed_freqs + 51;
    Binmsg=[];
    len = length(signal);
    for i = 1:len/100
        y=fftshift(fft(signal((i-1)*100+1:(i)*100)));
        k=max(abs(y));
        y=y/k;
      % disp([i , abs(y(31)), abs(y(6))]);

        for j = 1:2^speed
            if (abs(y(Fourier(j)))>0.9)
                
                binJ = dec2bin(j-1,speed);
             %   disp([j,binJ]);
                Binmsg = [Binmsg binJ];
            end
        end
    end
    bitcount = length(Binmsg);
    finalmsg = [];
    load("Mapset.mat");
    for i =1:(bitcount/5)
        subl = Binmsg(5*(i-1)+1:5*(i-1)+5);
        
        jdx = find(strcmp(data(2, :), subl));
        letter = data{1, jdx}; 
        finalmsg = [finalmsg letter];    
        
    end
    decoded = finalmsg;
end