function myTracker()
    [filename, pathname] = uigetfile({'*.mp4;*.avi'}, 'Select an IR video file');
    if isequal(filename,0)
        return;
    end
    videoFile = fullfile(pathname, filename);
    vid = VideoReader(videoFile);

    numBgFrames = 10;
    bg = zeros(vid.Height, vid.Width);
    for i = 1:numBgFrames
        if hasFrame(vid)
            frame = readFrame(vid);
            frame = rgb2gray(frame);
            bg = bg + double(frame);
        end
    end
    bg = uint8(bg / numBgFrames);
    vid.CurrentTime = 0;

    figure;
    while hasFrame(vid)
        frame = readFrame(vid);
        gray = rgb2gray(frame);
        diffFrame = imabsdiff(gray, bg);
        diffFrame = medfilt2(diffFrame, [5 5]);
        bw = diffFrame > 40;
        bw = imopen(bw, strel('disk', 3));
        bw = imclose(bw, strel('disk', 10));
        bw = imfill(bw, 'holes');
        stats = regionprops(bw, 'BoundingBox', 'Area');
        if ~isempty(stats)
            [~, idx] = max([stats.Area]);
            bbox = stats(idx).BoundingBox;
            frame = insertShape(frame, 'Rectangle', bbox, 'Color', 'green', 'LineWidth', 2);
        end
        
        imshow(frame);
        title('Tracking Airplane...');
        drawnow;
    end
end