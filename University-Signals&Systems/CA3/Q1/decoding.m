function detected = decoding(coded_pic, mapset)
    block_size = 6;
    char_max_bit = 5;
    [rows, cols] = size(coded_pic);
    num_blocks_x = floor(cols / block_size);
    num_blocks_y = floor(rows / block_size);
    uncovered = '';

    row_sizes = ones(1, num_blocks_y) * block_size;
    col_sizes = ones(1, num_blocks_x) * block_size;
    all_blocks = mat2cell(coded_pic, row_sizes, col_sizes);

    variance_blocks = zeros(num_blocks_y, num_blocks_x);
    for yy = 1:num_blocks_y
        for xx = 1:num_blocks_x
            block = all_blocks{yy, xx};
            blockmsb = bitshift(block, -1);
            variance_blocks(yy, xx) = var(double(blockmsb(:)));
        end
    end

    [~, sorted_indices] = sort(variance_blocks(:), 'descend');
    ranked_variance_block = zeros(size(variance_blocks));
    ranked_variance_block(sorted_indices) = 1:numel(sorted_indices);

    total_blocks = num_blocks_y * num_blocks_x;
    most_req_bit = 8;
    bits = '';
    stop_flag = false;

    for w = 1:total_blocks
        [the_shape_row, the_shape_column] = find(ranked_variance_block == w);
        final_block = all_blocks{the_shape_row, the_shape_column};
        [block_rows, block_cols] = size(final_block);

        for c_r = 1:block_rows
            for c_c = 1:block_cols
                current_pixel = final_block(c_r, c_c);
                binarized_pixel = dec2bin(current_pixel, most_req_bit);
                bits = [bits binarized_pixel(end)];

                if length(bits) >= char_max_bit
                    segment = bits(end-char_max_bit+1:end);
                    if strcmp(segment, '11111')
                        stop_flag = true;
                        break;
                    end
                end
            end
            if stop_flag, break; end
        end
        if stop_flag, break; end
    end

    bits_len = length(bits);
    for q = 1:char_max_bit:bits_len-char_max_bit+1
        segment = bits(q:q+char_max_bit-1);
        foundValue = '';
        for i = 1:length(mapset(2, :))
            if strcmp(segment, mapset{2, i})
                foundValue = mapset{1, i};
                break;
            end
        end
        if ~isempty(foundValue)
            uncovered = [uncovered foundValue];
        end
    end

    detected = append(uncovered, ';');
end
