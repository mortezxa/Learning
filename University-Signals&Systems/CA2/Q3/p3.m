clc ; close all ; clear ;
[file , path] = uigetfile({'*.jpg;*.bmp;*.png;*.tif'} , 'choose your car''s plate image');
picture_details = [path , file];
pic = imread(picture_details);
pic = imresize(pic , [300 , 500]);
sample = imread('sample.png');
sample = imresize(sample , [30 , 18]);
R_sample = sample(:,:,1);
G_sample = sample(:,:,2);
B_sample = sample(:,:,3);
R_pic = pic(:,:,1);
G_pic = pic(:,:,2);
B_pic = pic(:,:,3);
R_corr = normxcorr2(R_sample , R_pic);
G_corr = normxcorr2(G_sample , G_pic);
B_corr = normxcorr2(B_sample , B_pic);
Final_corr = (R_corr + G_corr + B_corr);
[m_val, m_idx] = max(Final_corr(:));
[max_row, max_column] = ind2sub(size(Final_corr), m_idx);
ymin_row = max_row - size(sample,1);
xmin_column = max_column - size(sample,2);
if m_val>=1.6 && m_val<=2
    plateID = pic(ymin_row:max_row+10 , xmin_column-5:17*size(sample , 2)+12 , 1:3);
else
    plateID = pic(ymin_row-5:max_row+10 , xmin_column-5:19*size(sample , 2)+12 , 1:3);
end
figure
subplot(1,2,1);
imshow(plateID);
plateID = rgb2gray(plateID);
plateID = imresize(plateID , [300 , 500]);
threshold = graythresh(plateID);
plateID = ~imbinarize(plateID , threshold);
subplot(1,2,2);
imshow(plateID);
plateID1 = bwareaopen(plateID , 600);
figure
subplot(1,3,1);
imshow(plateID1);
plateID2 = bwareaopen(plateID , 5000);
subplot(1,3,2);
imshow(plateID2);
plateID = plateID1 - plateID2;
subplot(1,3,3);
imshow(plateID);
[L , num] = bwlabel(plateID);
load PersianMapset;
len_mapset = size(PMap, 2);
plateIDnum = [];
for i=1:num
  [row , col] = find(L==i);
  curr_segment = plateID(min(row):max(row) , min(col):max(col));
  curr_segment = imresize(curr_segment ,[150 , 90]);
  ro = zeros(1,len_mapset);
  for j=1:len_mapset
      ro(j)=corr2(curr_segment , PMap{1,j});
  end
  [M_ro , P_ro] = max(ro);
  if M_ro>=0.45
    if ro(3)>=0.79 && ro(4)>=0.67
         out = '3';
    else
         out = cell2mat(PMap(2 , P_ro));
    end
    if i == 7
        plateIDnum = [plateIDnum '|' out];
    else
        plateIDnum = [plateIDnum ' ' out];
    end
  end
end
plateIDnum = fliplr(plateIDnum);
fprintf('The plate ID is :\n%s\n',plateIDnum);
file = fopen('ID_Plate.txt', 'wt');
fprintf(file,'%s\n',plateIDnum);
fclose(file);
winopen('ID_Plate.txt')