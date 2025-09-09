# llm.py
import os
import json
import requests
from dotenv import load_dotenv
from openai import OpenAI
from colorama import Fore, Style, init

init(autoreset=True)

load_dotenv()
OLLAMA_URL = "http://localhost:11434/api/generate"
OLLAMA_MODEL = "mistral"
OPENAI_MODEL = "gpt-3.5-turbo"

client = OpenAI(api_key=os.getenv("OPENAI_API_KEY"))

def build_prompt(fingerprint, file_list):
    return f"""
You are assisting a red team during an active operation.

They have scanned a compromised machine with this fingerprint:

{json.dumps(fingerprint, indent=2)}

The following files were discovered:

{json.dumps(file_list, indent=2)}

Your job:
- Identify which files may help achieve red team objectives (e.g., credentials, configurations, tokens, certificates, screenshots, budgets).
- Prioritize files related to privilege escalation, lateral movement, persistence, or exfiltration.

Respond ONLY in JSON like this:
{{
  "exfiltrate": ["file1", "file2"],
  "reasons": {{
    "file1": "Contains credentials for VPN or internal apps.",
    "file2": "Likely stores screenshots or sensitive documents."
  }}
}}
"""

def parse_response_to_json(response_text):
    try:
        return json.loads(response_text)
    except Exception as e:
        print(f"{Fore.RED}[❌] Failed to parse JSON:{Style.RESET_ALL}", str(e))
        return {"exfiltrate": [], "reasons": {}}

def call_ollama(prompt):
    try:
        res = requests.post(OLLAMA_URL, json={"model": OLLAMA_MODEL, "prompt": prompt, "stream": False})
        res.raise_for_status()
        result = res.json()["response"]
        print(f"{Fore.GREEN}[✅] Ollama responded.")
        return parse_response_to_json(result)
    except Exception as e:
        print(f"{Fore.YELLOW}[⚠️] Ollama failed:{Style.RESET_ALL}", str(e))
        return None

def call_openai(prompt):
    try:
        res = client.chat.completions.create(
            model=OPENAI_MODEL,
            messages=[
                {"role": "system", "content": "You are a cybersecurity red team assistant."},
                {"role": "user", "content": prompt}
            ],
            temperature=0.2
        )
        result = res.choices[0].message.content
        print(f"{Fore.GREEN}[✅] OpenAI responded.")
        return parse_response_to_json(result)
    except Exception as e:
        print(f"{Fore.RED}[❌] OpenAI failed:{Style.RESET_ALL}", str(e))
        return {"exfiltrate": [], "reasons": {}}

def get_llm_decision(fingerprint, file_list):
    prompt = build_prompt(fingerprint, file_list)

    ollama_result = call_ollama(prompt)
    if ollama_result:
        return ollama_result

    print(f"{Fore.BLUE}[↩️] Falling back to OpenAI API...{Style.RESET_ALL}")
    return call_openai(prompt)