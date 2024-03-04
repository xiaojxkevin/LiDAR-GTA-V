import os
import numpy as np
import open3d as o3d
from multiprocessing import Pool
from scipy.spatial.transform import Rotation as R

global poses
poses = np.genfromtxt("./poses.txt", dtype=np.float32)

def origin(args):
    i, scan_name, save_name = args

    pose = poses[i]
    center = pose[1:4]
    rotation = R.from_euler("xyz", pose[4:], degrees=True).as_matrix()
    pcd = o3d.io.read_point_cloud(scan_name)
    points = np.asarray(pcd.points, dtype=np.float32)
    # print(center)
    # print(points[:2])
    points = (points - center) @ (np.linalg.inv(rotation).T)
    # points = (points - center) @ rotation
    # print(points[:2])
    pcd.points = o3d.utility.Vector3dVector(points)
    # o3d.visualization.draw_geometries([pcd], 
    #                               point_show_normal=True)
    o3d.io.write_point_cloud(save_name, pcd)

if __name__ == "__main__":
    scan_dir = r"D:\Games\data\save_maps\clean"
    names = [n[:-4] for n in sorted(os.listdir(scan_dir))]
    scan_names = [os.path.join(scan_dir, "{}.pcd".format(n)) for n in names]
    # origin((1, scan_names[0], "./ori_0.pcd"))
    save_dir = r"D:\Games\data\save_maps\clean_zero"
    save_names = [os.path.join(save_dir, "{}.pcd".format(n)) for n in names]
    ids = range(len(names))
    with Pool(10) as pool:
        pool.map(origin, zip(ids, scan_names, save_names))
