clc; clear; close all;
[fname, fpath] = uigetfile({'*.jpg;*.bmp;*.png;*.tif'}, 'Select a plate image');
if isequal(fname,0)
    disp('No file selected');
    return;
end
imgPath = fullfile(fpath, fname);
origImg = imresize(imread(imgPath), [300 500]);
figure(1);
subplot(1,4,1);
imshow(origImg);
title('Original');

grayImg = mygrayfun(origImg);
subplot(1,4,2);
imshow(grayImg);
title('Gray');

th = 0.3902;
bwImg = ~mybinaryfun(grayImg, th);
subplot(1,4,3);
imshow(bwImg);
title('Binary');

bw_small = myremovecom(bwImg, 300);
bw_large = myremovecom(bwImg, 2500);
final_bw = bw_small - bw_large;
subplot(1,4,4);
imshow(final_bw);
title('Processed');

[L, N] = mysegmentation(final_bw);
load TRAININGSET.mat
numTemplates = size(TRAIN, 2);
plateResult = [];
for k = 1:N
    [r, c] = find(L == k);
    charImg = imresize(final_bw(min(r):max(r), min(c):max(c)), [42, 24]);
    corrVals = zeros(1, numTemplates);
    for t = 1:numTemplates
        corrVals(t) = corr2(charImg, TRAIN{1, t});
    end
    [maxCorr, idx] = max(corrVals);
    if maxCorr >= 0.43
        if corrVals(14) >= 0.75 && corrVals(51) >= 0.75
            recognizedChar = 'D';
        else
            recognizedChar = cell2mat(TRAIN(2, idx));
        end
        plateResult = [plateResult recognizedChar];
    end
end
fprintf('\nDetected Plate Number: %s\n', plateResult);
fid = fopen('Detected_Plate.txt', 'wt');
fprintf(fid, '%s\n', plateResult);
fclose(fid);
winopen('Detected_Plate.txt');

function [LabelMap, count] = mysegmentation(binImg)

[rPos, cPos] = find(binImg == 1);
pixels = [rPos'; cPos'];
LabelMap = zeros(size(binImg));
components = {};
idx = 1;

while ~isempty(pixels)
    startPix = pixels(:,1);
    pixels(:,1) = [];
    
    q = startPix;
    comp = startPix;
    
    while ~isempty(q)
        curR = q(1,1);
        curC = q(2,1);
        q(:,1) = [];
        
        for dR = -1:1
            for dC = -1:1
                if dR == 0 && dC == 0
                    continue;
                end
                
                newR = curR + dR;
                newC = curC + dC;
                
                if newR < 1 || newR > size(binImg,1) || ...
                   newC < 1 || newC > size(binImg,2)
                    continue;
                end
                
                if binImg(newR, newC) ~= 1
                    continue;
                end
                
                LabelMap(newR, newC) = idx;
                matchIdx = find(pixels(1,:) == newR & pixels(2,:) == newC);
                if ~isempty(matchIdx)
                    q = [q [newR; newC]];
                    comp = [comp [newR; newC]];
                    pixels(:, matchIdx) = [];
                end
            end
        end
    end
    
    components{end+1} = comp;
    idx = idx + 1;
end

count = idx - 1;
end

function filteredImg = myremovecom(binImg, minSize)
[r, c] = find(binImg == 1);
coords = [r'; c'];
connectedRegions = {};
while ~isempty(coords)
    startPt = coords(:,1);
    coords(:,1) = [];
    stack = startPt;
    region = startPt;
    while ~isempty(stack)
        curR = stack(1,1);
        curC = stack(2,1);
        stack(:,1) = [];
        for dr = -1:1
            for dc = -1:1
                if dr == 0 && dc == 0
                    continue;
                end
                newR = curR + dr;
                newC = curC + dc;
                if newR < 1 || newR > size(binImg,1) || newC < 1 || newC > size(binImg,2)
                    continue;
                end
                if binImg(newR, newC) == 1
                    matchIdx = find(coords(1,:) == newR & coords(2,:) == newC);
                    if ~isempty(matchIdx)
                        stack = [stack [newR; newC]];
                        region = [region [newR; newC]];
                        coords(:, matchIdx) = [];
                    end
                end
            end
        end
    end
    connectedRegions{end+1} = region;
end
filteredImg = zeros(size(binImg));
for k = 1:numel(connectedRegions)
    cur = connectedRegions{k};
    if size(cur, 2) >= minSize
        rr = cur(1,:);
        cc = cur(2,:);
        idx = sub2ind(size(binImg), rr, cc);
        filteredImg(idx) = 1;
    end
end
end
function gimg=mygrayfun(img)
r=img(:,:,1);
g=img(:,:,2);
b=img(:,:,3);
gimg=0.2989*r+0.5870*g+0.1140*b;
end

function outImg=mybinaryfun(img,th)
img=double(img)/255;
outImg=zeros(size(img));
outImg(img>=th)=1;
end

clc;
clear;
close all;
data=dir('Map Set');
n=numel(data)-2;
TRAIN=cell(2,n);
for k=1:n
    img=imread(fullfile(data(k+2).folder,data(k+2).name));
    TRAIN{1,k}=img;
    TRAIN{2,k}=data(k+2).name(1);
end
save('TRAININGSET','TRAIN');