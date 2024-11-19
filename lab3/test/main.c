#include <curl/curl.h>
#include <stdio.h>
#include <unistd.h>

size_t write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    // Обработка полученных данных
    printf("%.*s", (int)(size * nmemb), (char *)ptr);
    return size * nmemb;
}

int main(void) {
    CURL *curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 120L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 60L);
        curl_easy_setopt(curl, CURLOPT_URL, "http://kremlin.ru");

        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        sleep(60);

        curl_easy_cleanup(curl);
    }
    return 0;
}
