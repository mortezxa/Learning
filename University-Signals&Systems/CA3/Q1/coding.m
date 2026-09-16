function coded_pic = coding(secret, pic, Map)  

    block_size = 6;   
    [rows, cols] = size(pic);  
    num_blocks_x = floor(cols / block_size);  
    num_blocks_y = floor(rows / block_size);  
    secret_len = length(secret);  
    bin_secret = "";  


    for idx = 1:secret_len  
        current_char = secret(idx);   
        char_position = find(contains(Map(1, :), current_char));  
        if ~isempty(char_position)  
            bin_secret = bin_secret + Map{2, char_position(1)};  
        end  
    end  
    bin_secret = bin_secret + "11111";
    mat_bin_secret = char(bin_secret); 

    row_sizes = ones(1, num_blocks_y) * block_size;  
    col_sizes = ones(1, num_blocks_x) * block_size;   
    all_blocks = mat2cell(pic, row_sizes, col_sizes);   
    variance_blocks = zeros(num_blocks_y, num_blocks_x);   

    for yy = 1:num_blocks_y  
        for xx = 1:num_blocks_x  
            block = all_blocks{yy, xx};  
            block_msb = bitshift(block, -1);  
            variance_blocks(yy, xx) = var(double(block_msb(:)));  
        end  
    end  

    [~, sorted_indices] = sort(variance_blocks(:), 'descend');  
    ranked_variance_block = zeros(size(variance_blocks));              
    ranked_variance_block(sorted_indices) = 1:numel(sorted_indices);   

    total_blocks = num_blocks_y * num_blocks_x;  
    pic_copy = pic;    
    current_pos = 1;   
 
    for w = 1:total_blocks  
        [the_shape_row, the_shape_column] = find(ranked_variance_block == w);  
        final_block = all_blocks{the_shape_row, the_shape_column};  
        [block_shape_row_index, block_shape_column_index] = size(final_block);  

        for c_r = 1:block_shape_row_index  
            for c_c = 1:block_shape_column_index  
            
                if current_pos <= length(mat_bin_secret)  
                    current_pixel = final_block(c_r, c_c);  
                    binarized_pixel = dec2bin(current_pixel, 8);  
                    binarized_pixel(end) = mat_bin_secret(current_pos); 
                    pic_copy((the_shape_row-1) * block_size + c_r, ...  
                             (the_shape_column-1) * block_size + c_c) = bin2dec(binarized_pixel);  
                    current_pos = current_pos + 1; 
                else  
                    break;  
                end  
            end  
            if current_pos > length(mat_bin_secret)  
                break;  
            end  
        end   
        if current_pos > length(mat_bin_secret)  
            break; 
        end  
    end  

    coded_pic = pic_copy; 
end  