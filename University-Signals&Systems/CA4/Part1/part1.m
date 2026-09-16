clear all;
clc;
msge = 'signal';
sgn1 = coding_amp(msge , 1);
sgn2 = coding_amp(msge , 2);
sgn3 = coding_amp(msge , 3);
ss = decoding_amp(sgn2 , 2);
ss1 = decoding_amp(sgn3 , 3);
s2 = decoding_amp(sgn1 , 1);
fprintf('decoded message for 3 bit/sec variable is: %s\n: ' , ss1);



%%
ex = zeros(1,3000); 
ex = ex + randn(1,3000);
showplot(ex);
disp(mean(ex));
disp(var(ex));

%%
noised_sgn1 = sgn1 + 0.01*(randn(1,length(sgn1)));
noised_sgn2 = sgn2 + 0.01*(randn(1,length(sgn2)));
noised_sgn3 = sgn3 + 0.01*(randn(1,length(sgn3)));
noisedec1 =  decoding_amp(noised_sgn1 , 1);
noisedec2 =  decoding_amp(noised_sgn2 , 2);
noisedec3 =  decoding_amp(noised_sgn3 , 3);
%%
flag1 = 0;
flag2 = 0;
flag3 = 0;
for i =  20:2:150

    temp1noise = sgn1 + 0.01*i*(randn(1,length(sgn1)));
    temp2noise = sgn2 + 0.01*i*(randn(1,length(sgn2)));
    temp3noise = sgn3 + 0.01*i*(randn(1,length(sgn3)));
    dc1 = decoding_amp(temp1noise , 1);
    dc2 = decoding_amp(temp2noise , 2);
    dc3 = decoding_amp(temp3noise , 3);

    if flag3==0 && dc3 ~= "signal"
        fprintf('decoded message for 1 bit/sec with i= %d is: %s\n: ' , i,dc1);
        fprintf('decoded message for 2 bit/sec with i= %d is: %s\n' , i,dc2);
        fprintf('decoded message for 3 bit/sec with i= %d is: %s\n' , i,dc3);
        flag3=1;
    end
    if dc2 ~= "signal" && flag2==0
        fprintf('decoded message for 1 bit/sec with i= %d is: %s\n: ' , i,dc1);
        fprintf('decoded message for 2 bit/sec with i= %d is: %s\n' , i,dc2);
        fprintf('decoded message for 3 bit/sec with i= %d is: %s\n' , i,dc3);
        flag2=1;
    end
    if dc1 ~= "signal" && flag1==0
        fprintf('decoded message for 1 bit/sec with i= %d is: %s\n: ' , i,dc1);
        fprintf('decoded message for 2 bit/sec with i= %d is: %s\n' , i,dc2);
        fprintf('decoded message for 3 bit/sec with i= %d is: %s\n' , i,dc3);
        flag1=1;
    end
end
%%

function coded = coding_amp(msg, speed)
    load("Mapset.mat"); 

    len = strlength(msg);
    bitcount = 5 * len;
    binmsg = zeros(1, bitcount);
    
    k = 1;
    for i = 1:len
        ch = extractBetween(msg, i, i);  
        idx = find(strcmp(data(1, :), ch)); 
        

        binBits = data{2, idx}; 
        for p = 1:5

            binmsg(k) = binBits(p);
            k = k + 1;
        end
    end
    binmsg = binmsg-48;
   % coded = binmsg;



    block_count = bitcount / speed;
    ts = zeros(1, 100 * block_count);
    
    for i = 1:block_count

        bitblock = binmsg((i-1)*speed + 1 : i*speed);
        str = num2str(bitblock);          
        str(str == ' ') = [];      
        %disp(bitblock);

        amp_level = bin2dec(str); 
    

        amplitude = amp_level / (2^speed - 1);
    
        for j = 1:100
            ts((i-1)*100 + j) = amplitude * sin(2 * pi * (j-1) / 100);
        end
    end
    
    figure;
    plot(ts);
    title(['Signal Plot (Speed = ', num2str(speed), ')']);
    xlabel('Sample Index');
    ylabel('Amplitude');
    
    coded = ts;


end

function decoded = decoding_amp(signal , speed)
    len = length(signal);
    blockcount = len/100;
    cor_sample = zeros(1, 100);
    for i = 1:100
        cor_sample(i) = 2*sin(2*i/100*pi);
    end

   
    bitcount = blockcount*speed;
    corlist = zeros(1,blockcount);
    for i = 1:blockcount
        subt = signal((i-1)*100+1:100*i);
        cor = abs(subt*(cor_sample'));
        corlist(i)=cor;
    end

 %   disp(corlist);
    if speed == 1
        cor_value = round((corlist/100));
    else
        cor_value = round((corlist/100)*(2^speed-1));
    end
    binmsg=[];
 %   disp(cor_value);
    for i = 1:blockcount
        bin_part = dec2bin(cor_value(i),speed);
        binmsg = [binmsg bin_part];
    end
    
   % decoded=binmsg;
    finalmsg = [];
    load("Mapset.mat");
    for i =1:(bitcount/5)
        subl = binmsg(5*(i-1)+1:5*(i-1)+5);
        
        jdx = find(strcmp(data(2, :), subl));
        letter = data{1, jdx}; 
        finalmsg = [finalmsg letter];    
        
    end
    decoded = finalmsg;
%    disp(decoded);
end


function showplot(signal)
    figure;
    subplot(1,2,1);
    plot(signal);
    title(['Signal Plot']);
    xlabel('Sample Index (time)');
    ylabel('Amplitude');
    
    subplot(1,2,2); 
    histogram(signal, 50);       
    title('Histogram of signal');
    xlabel('Value');
    ylabel('Frequency');
    
end