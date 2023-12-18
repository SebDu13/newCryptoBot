#!/usr/bin/env python3

import logging
import requests
from bs4 import BeautifulSoup
from selenium import webdriver
from selenium.webdriver.chrome.service import Service as ChromeService
from selenium.webdriver.support.wait import WebDriverWait
import selenium.webdriver.support.expected_conditions as EC
from selenium.webdriver.common.by import By
import time
import subprocess
from datetime import datetime, timedelta
import pytz

def get_new_listing_currencies():
    try:
        response = requests.get("https://api.gateio.ws/api/v4/spot/currency_pairs")
        
        if response.status_code == 200:
            json = response.json()
            return [item["id"] for item in json if item["trade_status"] == "sellable"]
        else:
            print(f"The request failed with status code {response.status_code}")

    except Exception as e:
        print(f"An error occurred: {str(e)}")

#currency_pair format: BTC_USDT
def get_start_time(currency_pair, debug = False):
    try:
        # Set up the Chrome WebDriver
        chrome_service = ChromeService("/usr/bin/chromedriver")  # Replace with the path to your chromedriver
        options = webdriver.ChromeOptions()
        #options.add_argument('--headless')  # Run the browser in headless mode (no GUI)
        options.add_argument('--enable-javascript')
        #options.add_argument('--enable-features=NetworkService,NetworkServiceInProcess')
        #options.add_argument('user-agent=Your User Agent String')
        driver = webdriver.Chrome(service=chrome_service, options=options)
        
        # Wait for a few seconds to allow dynamic content to load (you might need to adjust the time)
        driver.implicitly_wait(10)
        
        # Open the URL in the browser
        driver.get(f"https://www.gate.io/trade/{currency_pair}")
        #WebDriverWait(driver, timeout=300).until(EC.frame_to_be_available_and_switch_to_it("iframe_name_or_id"))
        
        #time.sleep(5)
        #WebDriverWait(driver, 20).until(EC.frame_to_be_available_and_switch_to_it((By.CSS_SELECTOR,"iframe[title='Widget containing a Cloudflare security challenge']")))
        #WebDriverWait(driver, 20).until(EC.element_to_be_clickable((By.CSS_SELECTOR, "label.ctp-checkbox-label"))).click()
        #time.sleep(10)
        #driver.get(f"https://www.gate.io/trade/{currency_pair}")
        
        html_content = driver.page_source
        soup = BeautifulSoup(html_content, 'html.parser')
        
        if debug:
            print(soup)

        # Find the element containing "Start Time"
        start_time_element = soup.find('div', {'class': 'new-coin-open-time'})

        if start_time_element:
            start_time_value = start_time_element.text.strip()
            print(f"{currency_pair} start Time: {start_time_value}")
        else:
            print(f"No start Time not found for {currency_pair}.")

    except Exception as e:
        print(f"An error occurred: {str(e)}")

    finally:
        # Close the browser
        driver.quit()
        
def getTradingTime(currency_pair):
    result = subprocess.run(["../build/bin/CrypotBot", "--exchange", "Gateio", "--id", f"{currency_pair}", "--mode", "TradingTime"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    raw_trading_time = result.stdout.split("until ")[1] #output 2023-12-20T20:00:00.002+08:00[Asia/Shanghai]
    raw_trading_time = raw_trading_time.split("[")[0]
    local_time = datetime.strptime(raw_trading_time, "%Y-%m-%dT%H:%M:%S.%f%z")
    utc_time = local_time.astimezone(pytz.utc)
    return utc_time

#logging.basicConfig(level=logging.DEBUG)

print("Trading time:")
currency_pairs = get_new_listing_currencies()
if currency_pairs:
    for currency_pair in currency_pairs:
        print(f"{currency_pair}: {getTradingTime(currency_pair)}")