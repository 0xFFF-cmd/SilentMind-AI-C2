# llm.py — Local LLM (Ollama + Mistral) for Red Team File Exfil Decisions

import json
import requests

OLLAMA_URL = "http://localhost:11434/api/generate"
MODEL = "mistral"

def get_llm_decision(fingerprint, file_list):
    # Format file list as JSON string for better readability in prompt
    files_pretty = json.dumps(file_list, indent=2)

    prompt = f"""
You are assisting a red team during an active operation.

They have scanned a compromised machine with this fingerprint:

{json.dumps(fingerprint, indent=2)}

The following files were discovered:

{files_pretty}

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

    try:
        response = requests.post(
            OLLAMA_URL,
            json={
                "model": MODEL,
                "prompt": prompt,
                "stream": False
            }
        )
        response.raise_for_status()
        output = response.json()["response"]

        # Try to parse LLM output to structured JSON
        return json.loads(output)

    except Exception as e:
        print("[LLM ERROR] ❌", str(e))
        return {"exfiltrate": [], "reasons": {}}