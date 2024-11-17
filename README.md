# ST-LIB

It is recommended to set up an `STLIB_PATH` env variable pointint to the root directory where the ST-LIB is located so that other team tools work out of the box. Many tutorials exist on how to set an environment variable in different OSs, you should easily find one by googling it.

## Container Setup
To use it you must install [Dev Containers extension on VSCode](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) and [Podman](https://podman.io) (you can try to use docker, but it hasn't been tested), and set dev containers extension to use Podman (you can see how [here](https://code.visualstudio.com/remote/advancedcontainers/docker-options#_podman)). 
Then, you must configure podman machine to increase memory (by default is 2GB) to 4 - 8 GB (the more the better). This step can be done through the GUI or using command line like this: 
```sh
podman machine stop
podman machine set --cpus 2 --memory 4096
podman machine start
```

