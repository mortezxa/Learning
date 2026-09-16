clc ; close all ; clear
files = dir('Persian Map Set');
len = length(files)-2;
PMap = cell(2 , len);
for i =1:len
   PMap{1,i}=imread([files(i+2).folder,'\',files(i+2).name]);
   PMap{2,i}=files(i+2).name(1);
end
save PersianMapset PMap;