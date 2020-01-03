import cv2
import numpy as np
import time

t1 = time.time()
cap = []
for i in range(64):
    if i<10:
        cap.append(cv2.VideoCapture('received0'+str(i)+'.bin'))
    else:
        cap.append(cv2.VideoCapture('received'+str(i)+'.bin'))
out = cv2.VideoWriter('testvideo2.mp4', cv2.VideoWriter_fourcc(*'XVID'), 30, (960*2, 480*2),True)

FRAME = []
j = 0
while(cap[0].isOpened()):
    ret = []
    frame = []
    for i in range(64):
        ret0, frame0 = cap[i].read()
        ret.append(ret0)

        s = 0
        if ret0 == True:
            s = frame0.sum()
            if s == 0 and j>63:
                frame.append(FRAME[i])
            else:
                frame.append(frame0)
        else:
             frame.append(frame0)
        if j<=63:
            FRAME.append(frame0)
        elif j>63 and s>0:
            FRAME[i] = frame0
        j += 1
    if ret[0] == True:

        both0 = np.concatenate((frame[0], frame[1], frame[2], frame[3],frame[4], frame[5], frame[6], frame[7]), axis=1)
        both1 = np.concatenate((frame[8], frame[9], frame[10], frame[11], frame[12], frame[13], frame[14], frame[15]), axis=1)
        both2 = np.concatenate((frame[16], frame[17], frame[18], frame[19], frame[20], frame[21], frame[22], frame[23]), axis=1)
        both3 = np.concatenate((frame[24], frame[25], frame[26], frame[27], frame[28], frame[29], frame[30], frame[31]), axis=1)
        both4 = np.concatenate((frame[32], frame[33], frame[34], frame[35], frame[36], frame[37], frame[38], frame[39]), axis=1)
        both5 = np.concatenate((frame[40], frame[41], frame[42], frame[43], frame[44], frame[45], frame[46], frame[47]), axis=1)
        both6 = np.concatenate((frame[48], frame[49], frame[50], frame[51], frame[52], frame[53], frame[54], frame[55]), axis=1)
        both7 = np.concatenate((frame[56], frame[57], frame[58], frame[59], frame[60], frame[61], frame[62], frame[63]), axis=1)

        both = np.concatenate((both0, both1, both2, both3, both4, both5, both6, both7), axis=0)
        #print(time.time()-t1)
        frameM = cv2.resize(both, (960*2, 480*2))
        #cv2.imshow('Frame', frameM)
        out.write(frameM)

        #print('Here')
        #if cv2.waitKey(1) & 0xFF == ord('q'):
        #    break


    else:
        break

for i in range(64):
    cap[i].release()

out.release()

print('done')

cv2.waitKey(0)
cv2.destroyAllWindows()
