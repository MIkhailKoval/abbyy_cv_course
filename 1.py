import subprocess
import matplotlib.pyplot as plt

best_time = []
better_time = []
time = []

windows = range(101, 301, 20)
for window in windows:
    out = subprocess.check_output("./build/Sort test.bmp best " + str(window),
        shell = True)
    best_time.append(float(out.decode()))
    print(window, "best", best_time[-1])

for window in windows:
    out = subprocess.check_output("./build/Sort test.bmp better " + str(window),
        shell = True)
    better_time.append(float(out.decode()))
    print(window, "better", better_time[-1])

# for window in windows:
#     if window > 21: 
#         continue
#     out = subprocess.check_output("./build/Sort test.bmp naive " + str(window),
#         shell = True)
#     time.append(float(out.decode()))
#     print(window, "time", time[-1])

plt.figure(figsize=(16, 9))
plt.plot(windows, best_time, color = 'green')
plt.plot(windows, better_time, color = 'orange')
# plt.plot([1, 3, 5, 11, 15, 21], time, color = 'blue')
plt.show()