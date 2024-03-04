import os
import numpy as np
import open3d as o3d
from multiprocessing import Pool


def save2pcd(args):
    scan_name, save_name = args
    data = np.genfromtxt(scan_name, dtype=np.float32, skip_header=3)
    coordinates = data[:, :3]
    normals = data[:, 6:]
    mask = ~np.all(coordinates == 0, axis=1)
    coordinates = coordinates[mask]
    normals = normals[mask]

    pcd = o3d.geometry.PointCloud()
    pcd.points = o3d.utility.Vector3dVector(coordinates)
    pcd.normals = o3d.utility.Vector3dVector(normals)
    # pcd = pcd.farthest_point_down_sample(10000)
    o3d.io.write_point_cloud(save_name, pcd)

if __name__ == "__main__":
    base_dir = r"E:\GTAV\data_set"
    names = [n[:-4] for n in sorted(os.listdir(base_dir))]
    # print(names[:2])
    # assert False
    scan_names = [os.path.join(base_dir, "{}.txt".format(n)) for n in names]
    save_dir = r"D:\Games\data\save_maps\clean"
    save_names = [os.path.join(save_dir, "{}.pcd".format(n)) for n in names]
    with Pool(10) as pool:
        pool.map(save2pcd, zip(scan_names, save_names))
