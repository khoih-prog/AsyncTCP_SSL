## Contributing to AsyncTCP_SSL

### Reporting Bugs

Please report bugs in [AsyncTCP_SSL Issues](https://github.com/khoih-prog/AsyncTCP_SSL/issues) if you find them.

However, before reporting a bug please check through the following:

* [Existing Open Issues](https://github.com/khoih-prog/AsyncTCP_SSL/issues) - someone might have already encountered this.

If you don't find anything, please [open a new issue](https://github.com/khoih-prog/AsyncTCP_SSL/issues/new).

### How to submit a bug report

Please ensure to specify the following:

* Arduino IDE version (e.g. 1.8.19) or Platform.io version
* `ESP32` Core Version (e.g. ESP32 core v2.0.5)
* `ESP32` Board type (e.g. ESP32_DEV, ESP32_S2, ESP32_S3, ESP32_C3, etc.)
* Contextual information (e.g. what you were trying to achieve)
* Simplest possible steps to reproduce
* Anything that might be relevant in your opinion, such as:
  * Operating system (Windows, Ubuntu, etc.) and the output of `uname -a` for Linux / Ubuntu
  * Network configuration


### Example

```
Arduino IDE version: 1.8.19
ESP32 core v2.0.5
ESP32_DEV Module
OS: Ubuntu 20.04 LTS
Linux xy-Inspiron-3593 5.15.0-53-generic #59~20.04.1-Ubuntu SMP Thu Oct 20 15:10:22 UTC 2022 x86_64 x86_64 x86_64 GNU/Linux

Context:
I encountered a crash while using this library

Steps to reproduce:
1. ...
2. ...
3. ...
4. ...
```

---

### Sending Feature Requests

Feel free to post feature requests. It's helpful if you can explain exactly why the feature would be useful.

There are usually some outstanding feature requests in the [existing issues list](https://github.com/khoih-prog/AsyncTCP_SSL/issues?q=is%3Aopen+is%3Aissue+label%3Aenhancement), feel free to add comments to them.

---

### Sending Pull Requests

Pull Requests with changes and fixes are also welcome!

Please use the `astyle` to reformat the updated library code as follows (demo for Ubuntu Linux)

1. Change directory to the library GitHub

```
xy@xy-Inspiron-3593:~$ cd Arduino/xy/AsyncTCP_SSL_GitHub/
xy@xy-Inspiron-3593:~/Arduino/xy/AsyncTCP_SSL_GitHub$
```

2. Issue astyle command

```
xy@xy-Inspiron-3593:~/Arduino/xy/AsyncTCP_SSL_GitHub$ bash utils/restyle.sh
```

