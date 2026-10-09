# learning project - http server implementation in c++

written with my bare hands

# todo:

- [ ] implement running flag
- [x] extract response sending logic to a separate method
- [x] return 400 when needed
- [x] return 404 when needed
- [x] differ http methods in router.handlers
- [x] add body to HttpResponse (plain text)
- [ ] add epoll and better threading (currently the code is frozen until client connects and sends a request. if the client connects and doesn't send any request, other clients are added to the queue and are never connected)
