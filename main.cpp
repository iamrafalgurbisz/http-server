#include <iostream>

#include "http/http-request.hpp"
#include "http/http-response.hpp"
#include "router/router.hpp"
#include "server/server.hpp"

int main()
{
  Router router;

  router.get("/api",
             [](const HttpRequest&)
             {
               auto res = HttpResponse::Ok()
                              .body("Hello from the server!")
                              .header(HeaderKey::ContentLength, "20");

               return res;
             });

  router.post("/api",
              [](const HttpRequest&)
              {
                auto res = HttpResponse::Ok().body("Hello");

                return res;
              });

  router.head("/api",
              [](const HttpRequest&)
              {
                auto res = HttpResponse::Ok();

                return res;
              });

  Server server(router);

  try
  {
    server.start(3000);
  }
  catch (const std::exception& e)
  {
    std::cerr << "Server error: " << e.what() << '\n';

    return 1;
  }

  return 0;
}
