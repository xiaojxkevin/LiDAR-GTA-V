import os
import numpy as np
import open3d as o3d
from multiprocessing import Pool
from scipy.spatial.transform import Rotation


def get_scans(args):
    pose, scan_name = args
    pcd = o3d.io.read_point_cloud(os.path.join(r"D:\Games\data\save_maps\clean_zero", scan_name))
    scan = np.asarray(pcd.points, dtype=np.float32)
    R, t = pose[:3, :3].T, pose[:3, 3:].T
    transformed_scan = np.matmul(scan, R) + t
    return transformed_scan

def read_slam_poses(pose_path):
    data = np.loadtxt(pose_path, delimiter=' ')
    odom_num = len(data)

    slam_stamps, raw_slam_odoms = data[:, :1].flatten(), data[:, 1:]
    translations = raw_slam_odoms[:, :3]
    quaternions = raw_slam_odoms[:, 3:]
    rotations = Rotation.from_quat(quaternions).as_matrix()

    poses = np.zeros((odom_num, 4, 4))
    poses[:, :3, :3] = rotations
    poses[:, :3, 3] = translations
    poses[:, 3, 3] = 1.0

    return slam_stamps, poses

if __name__ == "__main__":
    global scan_dir

    # seq = 6
    slam = "kissicp"
    # time = "2024-02-28T10:28"
    # scan_dir = "/home/daizj/data/project/kiss-icp/python/Point-Cloud-Bias-Correction-xjx/data/processed/kitti360/seq{:02d}/sampled_scans".format(seq)
    # scan_dir = f"/home/daizj/data/project/kiss-icp/python/Point-Cloud-Bias-Correction-xjx/experiments/{time}/points"
    scan_dir = r"D:\Games\data\save_maps\clean_zero"
    scan_names = sorted(os.listdir(scan_dir))
    print(scan_names)

    # pose_path = "/home/daizj/data/project/kiss-icp/python/Point-Cloud-Bias-Correction-xjx/data/processed/kitti360/seq{:02d}/poses/{}_0_500_1__tum.txt".format(seq, slam)
    # pose_path = f"/home/daizj/data/project/kiss-icp/python/Point-Cloud-Bias-Correction-xjx/experiments/{time}/results/{slam}-tum.txt"
    pose_path = "D:\\mpl\\add_noise\\{}__tum.txt".format(slam)
    _, poses = read_slam_poses(pose_path)
    with Pool(10) as pool:
        scans = list(pool.map(get_scans, zip(poses, scan_names)))

    scans = np.concatenate(scans, axis=0)
    print(scans.shape)
    save_path = "./{}_map.txt".format(slam)
    np.savetxt(save_path, scans)
