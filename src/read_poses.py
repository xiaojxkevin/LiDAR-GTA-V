import os
import numpy as np
from scipy.spatial.transform import Rotation as R

if __name__ == "__main__":
    base_dir = r"E:\GTAV\data_set"
    scan_names = [os.path.join(base_dir, n) for n in sorted(os.listdir(base_dir))]
    save_path = r"D:\mpl\add_noise\poses.txt"
    poses = []
    for i, n in enumerate(scan_names):
        with open(n, "r") as f:
            x, y, z = f.readline().split(" ")[1:]
            center = np.array([float(x), float(y), float(z)], dtype=np.float32)
            rx, ry, rz = f.readline().split(" ")[1:]
            rotation = np.array([float(rx), float(ry), float(rz)], dtype=np.float32)
            pose = np.concatenate([[i], center, rotation])
            poses.append(pose)
            f.close()

    poses = np.asarray(poses, dtype=np.float32)
    np.savetxt(save_path, poses)

