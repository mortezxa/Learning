clear all;
Mapset = cell(2,32);
for i = 0:25  
    Mapset{1, i+1} = char('a' + i);
    Mapset{2, i+1} = dec2bin(i, 5);   
end  
specialChars = ' .,!";'; 
for i = 0:5 
    Mapset{1, i+27} = specialChars(i+1);
    Mapset{2, i+27} = dec2bin(i+26, 5);   
end  

%Mapset{1, 27:32} = specialChars;        
%Mapset{2, 27:32} = dec2bin(specialChars, 8);    
img = imread('heu.jpg');
grayImg = rgb2gray(img);  
block_size = 6;
new_rows = (size(grayImg, 1) + block_size - 1) - mod(size(grayImg, 1) - 1, block_size);  
new_cols = (size(grayImg, 2) + block_size - 1) - mod(size(grayImg, 2) - 1, block_size);  
pic = imresize(grayImg, [new_rows new_cols]);  
hidden = 'signal;';    
coded_picture = coding(hidden, pic, Mapset);
figure;

%difference_image = abs(double(pic) - double(coded_picture));  
subplot(1, 2, 1);
imshow(pic);
title('Normal picture');
subplot(1, 2, 2);
imshow(coded_picture);
title('Coded picture');
message = decoding(coded_picture, Mapset);
disp(message);
