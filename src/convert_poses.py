import os
import numpy as np
from scipy.spatial.transform import Rotation as R

if __name__ == "__main__":
    pose_path = "./poses.txt"
    poses = np.genfromtxt(pose_path, dtype=np.float32)
    new_poses = []
    for pose in poses:
        idx = pose[0]
        center = pose[1:4]
        angles = pose[4:]
        p = np.eye(4, dtype=np.float32)
        p[:3, 3] = center
        p[:3, :3] = R.from_euler("xyz", angles, degrees=True).as_matrix()
        new_poses.append(p)

    origin = np.linalg.inv(new_poses[0])
    new_poses = [origin @ p for p in new_poses]
    # new_poses_path = "./gt_poses.npy"
    # new_poses = np.asarray(new_poses, dtype=np.float32)
    # np.save(new_poses_path, new_poses)
    poses = []
    for i, pose in enumerate(new_poses):
        t = pose[:3, 3]
        r = R.from_matrix(pose[:3, :3]).as_quat()
        poses.append(np.concatenate([[i], t, r], dtype=np.float32))
    poses = np.asarray(poses, dtype=np.float32)
    np.savetxt("./gt__tum.txt", poses)

