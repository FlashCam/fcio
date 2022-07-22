build_images = {
    "Debian 9": "debian-9-devtools",
    "Ubuntu 18.04": "ubuntu-18.04-devtools",
    "CentOS 7": "centos-7-devtools",
    "CentOS 8": "centos-8-devtools",
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
