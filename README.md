### Brief

asio的几种常见用法，我们的目标是没有蛀～呸呸呸～是正确、高效、易用、易读、易维护～

### What do chat-demo do?

* server接受client的消息并广播给其它client。
* 新client连接成功后，server主动发送(max(100,n))条历史消息给新client。
* [optional] server定期清理一段时间内（10seconds e.g）没有上行数据的链接。

### Why chat-demo?

很多echo或http-demo，数据都是读上来后本地做些简单操作就在当前io线程发送出去了～

不同于它们，chat-demo链接与链接之间存在link\unlink\send\broadcast等操作，管理会更复杂些。

| app  | client-interactive-with-others |
| ---  | --- |
| echo | nope |
| http | interactive by other apps,such as database |
| chat | memory |

### Worth mentioning

* O(n*n)的转发量（尽管转发n的时候优化为maintain同一份内存，但是client不收堵在kernel各自的tcp sendbuf也只能摊手～），注意点不要爆内存了～如果程序被Killed了，可以dmesg确认下是不是爆内存被kernel干了～

* statistics类为了通用使用了atomic，在server1中是非必须的，其他所有的锁及位置都是一个不多，一个不少 :)

* 精准控制所有对象及buffer的生命周期管理，不早不晚～

* 接收结束信号后优雅的释放所有资源再退出程序，从此跑valgrind定位义务层内存泄漏so easy了有木有

### server1

Single io loop @ a single thread.

### server2

Leader-Follower. Only one io service. All threads are io threads.

### server3

Multi io service. One loop per thread.

### client_stdin

主线程接受控制台的输入，io线程处理io。

演示如何通过io_service post在非io线程中不加锁发送消息(全部post到connection所属的io线程中做同步)。

相当于间接演示了work thread和io thread的交互。

### client_benchmark

启动一定数量（512 e.g）的client连接同一chat server，主动发送两条固定长度（'HELLO' 'WORLD' e.g）的消息，接受历史消息，并接受其他client的消息。

### Run

```
# my env
ubuntu 14.04.4 LTS x86_64
gcc version 4.8.4

# tools & deps
use apt-get install these:
scons
boost

# build
$scons

# run server2 at port 5566 with 16 thread
$cd build/server2 && ./server2 5566 16

# run client_stdin to port 5566
$cd build/client_stdin && ./client_stdin 127.0.0.1 5566

# run client_benchmark, start 512 connection to port 5566
$ cd build/client_benchmark && ./client_benchmark 127.0.0.1 5566 512
.
.
.
# there were also some run.sh and valgrind.sh in build/
```

### TODO

