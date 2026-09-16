clc ; clear ; close all ; 
video = VideoReader('myvideo.mp4');
t1 = 1.7;
t2 = 3.1;
video.CurrentTime = t1;
frame1 = readFrame(video);
subplot(1,2,1);
imshow(frame1);
video.CurrentTime = t2;
frame2 = readFrame(video);
subplot(1,2,2);
imshow(frame2);
sample = imread('sample.png');
frame1 = imresize(frame1 , [NaN , 1000]);
frame2 = imresize(frame2 , [NaN , 1000]);
sample1 = imresize(sample , [60 , 30]);
sample2 = imresize(sample , [25 , 15]);
R_sample1 = sample1(:,:,1);
G_sample1 = sample1(:,:,2);
B_sample1 = sample1(:,:,3);
R_frame1 = frame1(:,:,1);
G_frame1 = frame1(:,:,2);
B_frame1 = frame1(:,:,3);
R_corr1 = normxcorr2(R_sample1 , R_frame1);
G_corr1 = normxcorr2(G_sample1 , G_frame1);
B_corr1 = normxcorr2(B_sample1 , B_frame1);
Final_corr1 = (R_corr1 + G_corr1 + B_corr1);
[m_val1, m_idx1] = max(Final_corr1(:));
[max_row1, max_column1] = ind2sub(size(Final_corr1), m_idx1);
ymin_row1 = max_row1 - size(sample1,1);
xmin_column1 = max_column1 - size(sample1,2);
x1 = frame1(ymin_row1:max_row1, xmin_column1:max_column1 , 1:3);
figure
imshow(x1)
R_sample2 = sample2(:,:,1);
G_sample2 = sample2(:,:,2);
B_sample2 = sample2(:,:,3);
R_frame2 = frame2(:,:,1);
G_frame2 = frame2(:,:,2);
B_frame2 = frame2(:,:,3);
R_corr2 = normxcorr2(R_sample2 , R_frame2);
G_corr2 = normxcorr2(G_sample2 , G_frame2);
B_corr2 = normxcorr2(B_sample2 , B_frame2);
Final_corr2 = (R_corr2 + G_corr2 + B_corr2);
[m_val2, m_idx2] = max(Final_corr2(:));
[max_row2, max_column2] = ind2sub(size(Final_corr2), m_idx2);
ymin_row2 = max_row2 - size(sample2,1);
xmin_column2 = max_column2 - size(sample2,2);
x2 = frame2(ymin_row2:max_row2 , xmin_column2:max_column2 , 1:3);
figure
imshow(x2)
distance1_pixel = size(frame1 , 1) - max_row1;
distance2_pixel = size(frame2 , 1) - max_row2; 
delta_t_hour = (t2-t1)/3600;
scale = 3.5*10^-5;
Displacement_km = (distance2_pixel-distance1_pixel)*scale;
v = Displacement_km/delta_t_hour;
disp(v)



