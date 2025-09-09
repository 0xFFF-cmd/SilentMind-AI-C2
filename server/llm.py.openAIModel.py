# llm.py — Uses OpenAI v1.x client to decide exfil targets
import os
from openai import OpenAI
from dotenv import load_dotenv

load_dotenv()  # Load API key from .env file

client = OpenAI(api_key=os.getenv("OPENAI_API_KEY"))

def get_llm_decision(fingerprint, file_list):
    prompt = f"""
You are helping a red teamer during an active engagement.

The red team has scanned a host with the following fingerprint:

    {fingerprint}

They discovered these local files:

    {file_list}

From a red team perspective:
- Which of these files are worth exfiltrating?
- Which ones contain credentials, configurations, access tokens, or other valuable data?
- Which could support privilege escalation, lateral movement, or final objective?

Reply ONLY in this JSON format:
{{
  "exfiltrate": [filenames to exfil],
  "reasons": {{
    "filename1": "Why it's valuable",
    "filename2": "Why it's valuable"
  }}
}}
"""

    response = client.chat.completions.create(
        model="gpt-4",
        messages=[
            {"role": "system", "content": "You are a cybersecurity red team assistant."},
            {"role": "user", "content": prompt}
        ],
        temperature=0.2
    )

    return response.choices[0].message.content
