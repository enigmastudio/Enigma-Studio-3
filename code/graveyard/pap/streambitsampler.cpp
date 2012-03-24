static const eU32 eMAX_UINT         = (1 << 31);



class eStreamBitSampler
{
public:
	eStreamBitSampler(eDataStream& stream)
		: m_stream(stream) {
	}

#ifdef eEDITOR
	eArray<eU32>	m_samples;
	eU32 m_bitsUnaligned;

	void sampleValue(eU32 value);
	void finishSampling();
	void writeHeader();
	void writeAll();
	void writeSamples();
	void write(eU32 value);
	void finishAndWriteAll();
	void finishAndWriteHeader();
#endif
	void			readHeader();
	eU32			read();
	eArray<eU32>	read(eU32 cnt);
	static	eArray<eU32>	readAll(eDataStream& stream, eU32 count);

private:
	eU32 m_minimum;
	eU32 m_step;
	eU32 m_bits;
	eDataStream&	m_stream;
};



#ifdef eEDITOR
void
eStreamBitSampler::sampleValue(eU32 value) {
	this->m_samples.append(value);
}

void
eStreamBitSampler::finishSampling() {
	this->m_minimum = 0;
	this->m_step = 0;
	this->m_bits = 0;
	this->m_bitsUnaligned = 0;
	if(this->m_samples.isEmpty())
		return;

	m_minimum = eMAX_UINT;
	for(eU32 i = 0; i < m_samples.size(); i++)
		m_minimum = eMin(m_minimum, m_samples[i]);
	
	eU32 minStep = eMAX_UINT;
	for(eU32 i = 0; i < m_samples.size(); i++) {
		eU32 step = m_samples[i] - m_minimum;
		if((step != 0) && (step < minStep))
			minStep = step;
	}
	if(minStep == eMAX_UINT)
		return;		// all sample values are equal

	// test step size
	m_step = minStep;
	eBool allMultiple = false;
	while(!allMultiple) {
		allMultiple = true;			
		for(eU32 i = 0; i < m_samples.size(); i++) {
			if(((m_samples[i] - m_minimum) % m_step) != 0) {
				allMultiple = false;
				break;
			}
		}
		if(!allMultiple)
			m_step = m_step >> 1;
	}
	// now all samples are multiples of m_step
	eU32 maxSaveVal = 0;
	for(eU32 i = 0; i < m_samples.size(); i++) 
		maxSaveVal = eMax(maxSaveVal, (m_samples[i] - m_minimum) / m_step);
		
	// calc bit count
	m_bits = 0;
	while(maxSaveVal != 0) {
		m_bits++;
		maxSaveVal = maxSaveVal >> 1;
	}
	// align bits to bytes
	this->m_bitsUnaligned = m_bits;
	if(m_bits != 0) m_bits = (((m_bits - 1) >> 3) << 3) + 8;
}

void 
eStreamBitSampler::writeHeader() {
	m_stream.writeBits(m_minimum, 32);
	m_stream.writeBits(m_step, 32);
	m_stream.writeBits(m_bits, 32);
	m_stream.writeBits(m_bits, 32);
}

void 
eStreamBitSampler::write(eU32 value) {
	if(this->m_step != 0)
		m_stream.writeBits((value - this->m_minimum) / this->m_step, this->m_bits);
}

void 
eStreamBitSampler::writeSamples() {
	for(eU32 i = 0; i < m_samples.size(); i++) 
		this->write(m_samples[i]);
}

void 
eStreamBitSampler::writeAll() {
	this->writeHeader();
	this->writeSamples();
}

void 
eStreamBitSampler::finishAndWriteAll() {
	this->finishSampling();
	this->writeAll();
}

void 
eStreamBitSampler::finishAndWriteHeader() {
	this->finishSampling();
	this->writeHeader();
}


#endif

void 
eStreamBitSampler::readHeader() {
	m_minimum = m_stream.readBits(32);
	m_step = m_stream.readBits(32);
	m_bits = m_stream.readBits(32);
	m_bits = m_stream.readBits(32);
}

eU32
eStreamBitSampler::read() {
	if(this->m_bits == 0)
		return this->m_minimum;
	else
		return this->m_minimum + this->m_step * m_stream.readBits(this->m_bits);
}

eArray<eU32>	
eStreamBitSampler::read(eU32 cnt) {
	eArray<eU32> result;
	for(eU32 i = 0; i < cnt; i++)
		result.append(this->read());
	return result;
}

eArray<eU32>	
eStreamBitSampler::readAll(eDataStream& stream, eU32 count) {
	eStreamBitSampler sampler(stream);
	sampler.readHeader();
	return sampler.read(count);
}