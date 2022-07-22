build_images = {
    "Debian 9": "debian-9-devtools",
    "Debian 10": "debian-10-devtools",
    "Debian 11": "debian-11-devtools",
    "Ubuntu 18.04": "ubuntu-18.04-devtools",
    "Ubuntu 20.04": "ubuntu-20.04-devtools",
    "Ubuntu 22.04": "ubuntu-22.04-devtools",
    "CentOS 7": "centos-7-devtools",
    "CentOS 8": "centos-8-devtools",
    "Rocky Linux 8": "rockylinux-8-devtools",
    "Rocky Linux 9": "rockylinux-9-devtools",
}


def build_fcio(name, image):
    return {
        "name": "build on " + name,
        "image": image,
        "pull": "if-not-exists",
        "depends_on": [],
        "commands": [
            'make',
        ]
    }


def main(ctx):
    pipelines = []
    for name, image in build_images.items():
        pipelines.append({"kind": "pipeline",
                          "type": "docker",
                          "name": "Build on " + name,
                          "steps": [build_fcio(name, image)]})
    return pipelines
