R_true = 450;
r_clean = alpha * double(t >= td & t <= td + tau);

noise_levels = 0:0.05:5; % قدرت نویز از 0 تا 0.5
num_trials = 100;           % تعداد تکرار برای هر سطح نویز
mean_errors = zeros(size(noise_levels));

for n = 1:length(noise_levels)
    noise_amp = noise_levels(n);
    errors = zeros(1,num_trials);
    for k = 1:num_trials
        noise = noise_amp * randn(size(r_clean)); % نویز گاوسی
        r_noisy = r_clean + noise;
        [c, lags] = xcorr(r_noisy, s);
        [~, idx] = max(c);
        td_est = lags(idx) * ts;
        R_est = C * td_est / 2;
        errors(k) = abs(R_est - R_true); % خطای فاصله
    end
    mean_errors(n) = mean(errors);
end

figure;
plot(noise_levels, mean_errors, '-o','LineWidth',1.5);
xlabel('Noise Amplitude');
ylabel('Mean Distance Error (m)');
title('Effect of Noise on Radar Distance Estimation');
grid on;

hold on;
yline(10,'r--','Threshold 10 m');
legend('Mean Error','10 m Threshold');